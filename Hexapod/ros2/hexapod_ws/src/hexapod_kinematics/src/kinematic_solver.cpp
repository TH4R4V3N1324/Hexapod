#include "rclcpp/rclcpp.hpp"
#include "hexapod_interfaces/srv/ik_solver.hpp"
#include "hexapod_interfaces/srv/fk_solver.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "geometry_msgs/msg/pose_array.hpp"
#include <unordered_map>

using std::placeholders::_1;
using std::placeholders::_2;

using IKSolver = hexapod_interfaces::srv::IKSolver;
using FKSolver = hexapod_interfaces::srv::FKSolver;

class KinematicSolver : public rclcpp::Node{
public: 
    KinematicSolver() : Node("kinematic_solver"){
        ik_service = this->create_service<IKSolver>("ik_solver", std::bind(&KinematicSolver::handle_ik_request, this, _1, _2));
        fk_service = this->create_service<FKSolver>("fk_solver", std::bind(&KinematicSolver::handle_fk_request, this, _1, _2));
        joint_state_sub = this->create_subscription<sensor_msgs::msg::JointState>("joint_states", 10, std::bind(&KinematicSolver::joint_state_callback, this, _1));
        foot_poses_pub = this->create_publisher<geometry_msgs::msg::PoseArray>("foot_poses", 10);
    }    
private:
    rclcpp::Service<IKSolver>::SharedPtr ik_service;
    rclcpp::Service<FKSolver>::SharedPtr fk_service;
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub;
    rclcpp::Publisher<geometry_msgs::msg::PoseArray>::SharedPtr foot_poses_pub;
    
    static constexpr double coxaLength  = 0.0505;   // meters
    static constexpr double femurLength = 0.090;    // meters
    static constexpr double tibiaLength = 0.15035;  // meters

    geometry_msgs::msg::Point compute_fk(double coxaAngle, double femurAngle, double tibiaAngle);

public:
    void handle_ik_request(const std::shared_ptr<IKSolver::Request> request, std::shared_ptr<IKSolver::Response> response);
    void handle_fk_request(const std::shared_ptr<FKSolver::Request> request, std::shared_ptr<FKSolver::Response> response);
    void joint_state_callback(const sensor_msgs::msg::JointState::SharedPtr msg);
};

int main(int argc, char **argv){    
    rclcpp::init(argc, argv);    
    auto node = std::make_shared<KinematicSolver>();    
    rclcpp::spin(node);    
    rclcpp::shutdown();    
    return 0;
}

void KinematicSolver::handle_ik_request(const std::shared_ptr<IKSolver::Request> request, std::shared_ptr<IKSolver::Response> response){
    // geometry_msgs/Point target --> sensor_msgs/JointState joint_state
    const auto &position = request->target;

    auto clamp = [](double v) {return std::max(-1.0, std::min(1.0, v));};

    double a1 = coxaLength;
    double a2 = femurLength;
    double a3 = tibiaLength;

    double coxaAngle = atan2(position.y, position.x);
    double r1 = std::hypot(position.x, position.y) - a1;
    double r2 = position.z;
    double q2 = atan(r2/r1);
    double r3 = std::hypot(r1, r2);
    double q1 = acos(clamp((pow(a3,2) - pow(a2,2) - pow(r3,2)) / (-2*a2*r3)));
    double femurAngle = (q2+q1);
    double q3 = acos(clamp((pow(r3,2) - pow(a2,2) - pow(a3,2)) / (-2*a2*a3)));
    double tibiaAngle = M_PI_2 - q3;

    std::string prefix = "leg" + std::to_string(request->leg_index + 1);

    response->joint_state.name = {
        prefix + "_coxa_joint",
        prefix + "_femur_joint",
        prefix + "_tibia_joint"
    };

    response->joint_state.position = {
        coxaAngle, 
        femurAngle, 
        tibiaAngle
    };
}

void KinematicSolver::handle_fk_request(const std::shared_ptr<FKSolver::Request> request, std::shared_ptr<FKSolver::Response> response){
    // sensor_msgs/JointState joint_state --> geometry_msgs/Point foot_position
    const auto &joint_state = request->joint_state;

    if (joint_state.position.size() < 3) {
        RCLCPP_ERROR(this->get_logger(), "FK request must have 3 joint angles");
        return;
    }

    geometry_msgs::msg::Point point = compute_fk(
        joint_state.position[0],
        joint_state.position[1],
        joint_state.position[2]
    );

    response->foot_position = point;
}

void KinematicSolver::joint_state_callback(const sensor_msgs::msg::JointState::SharedPtr msg){
    // Expect 18 joints: 6 legs × 3 joints (coxa, femur, tibia)
    // Joint naming: leg1_coxa_joint, leg1_femur_joint, leg1_tibia_joint, leg2_..., etc.
    
    if (msg->position.size() < 18) {
        return;  // Not enough joint data
    }

    geometry_msgs::msg::PoseArray pose_array;
    pose_array.header = msg->header;
    pose_array.header.frame_id = "base_link";

    // Build a map from joint name to position for flexible ordering
    std::unordered_map<std::string, double> joint_map;
    for (size_t i = 0; i < msg->name.size() && i < msg->position.size(); ++i) {
        joint_map[msg->name[i]] = msg->position[i];
    }

    // Process each of the 6 legs
    for (int leg = 1; leg <= 6; ++leg) {
        std::string prefix = "leg" + std::to_string(leg);
        
        auto coxa_it = joint_map.find(prefix + "_coxa_joint");
        auto femur_it = joint_map.find(prefix + "_femur_joint");
        auto tibia_it = joint_map.find(prefix + "_tibia_joint");
        
        if (coxa_it == joint_map.end() || femur_it == joint_map.end() || tibia_it == joint_map.end()) {
            RCLCPP_WARN_ONCE(this->get_logger(), "Missing joints for %s", prefix.c_str());
            continue;
        }

        geometry_msgs::msg::Point foot_pos = compute_fk(
            coxa_it->second, femur_it->second, tibia_it->second);

        geometry_msgs::msg::Pose pose;
        pose.position = foot_pos;
        pose.orientation.w = 1.0;  // Identity quaternion
        pose_array.poses.push_back(pose);
    }

    foot_poses_pub->publish(pose_array);
}

geometry_msgs::msg::Point KinematicSolver::compute_fk(double coxaAngle, double femurAngle, double tibiaAngle){
    double a1 = coxaLength;
    double a2 = femurLength;
    double a3 = tibiaLength;

    double tibiaAbsoluteAngle = femurAngle - tibiaAngle - M_PI/2.0;

    double femurVertical = a2 * sin(femurAngle);
    double femurHorizontal = a2 * cos(femurAngle);
    double tibiaVertical = a3 * sin(tibiaAbsoluteAngle);
    double tibiaHorizontal = a3 * cos(tibiaAbsoluteAngle);
    
    double z = femurVertical + tibiaVertical;
    double horizontalReach = a1 + femurHorizontal + tibiaHorizontal;
    double x = horizontalReach * cos(coxaAngle);
    double y = horizontalReach * sin(coxaAngle);

    geometry_msgs::msg::Point point;
    point.x = x;
    point.y = y;
    point.z = z;

    return point;
}