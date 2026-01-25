#include "rclcpp/rclcpp.hpp"
#include "hexapod_interfaces/srv/ik_solver.hpp"
#include "hexapod_interfaces/srv/fk_solver.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

using IKSolver = hexapod_interfaces::srv::IKSolver;
using FKSolver = hexapod_interfaces::srv::FKSolver;

class KinematicSolver : public rclcpp::Node{public: KinematicSolver() : Node("kinematic_solver"){
    ik_service = this->create_service<IKSolver>("ik_solver", std::bind(&KinematicSolver::handle_ik_request, this, _1, _2));
    fk_service = this->create_service<FKSolver>("fk_solver", std::bind(&KinematicSolver::handle_fk_request, this, _1, _2));
}    
private:
    rclcpp::Service<IKSolver>::SharedPtr ik_service;
    rclcpp::Service<FKSolver>::SharedPtr fk_service;
    static constexpr double coxaLength  = 0.0505;   // meters
    static constexpr double femurLength = 0.090;    // meters
    static constexpr double tibiaLength = 0.15035;  // meters
public:
    void handle_ik_request(const std::shared_ptr<IKSolver::Request> request, std::shared_ptr<IKSolver::Response> response);
    void handle_fk_request(const std::shared_ptr<FKSolver::Request> request, std::shared_ptr<FKSolver::Response> response);
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

    double a1 = coxaLength;
    double a2 = femurLength;
    double a3 = tibiaLength;

    double coxaAngle  = joint_state.position[0];  // radians
    double femurAngle = joint_state.position[1];  // radians
    double tibiaAngle = joint_state.position[2];  // radians

    double tibiaAbsoluteAngle = femurAngle - tibiaAngle - M_PI/2.0;

    // Vertical components (sin) and horizontal components (cos)
    double femurVertical = a2 * sin(femurAngle);
    double femurHorizontal = a2 * cos(femurAngle);
    double tibiaVertical = a3 * sin(tibiaAbsoluteAngle);
    double tibiaHorizontal = a3 * cos(tibiaAbsoluteAngle);
    
    double z = femurVertical + tibiaVertical;
    double horizontalReach = a1 + femurHorizontal + tibiaHorizontal;
    double x = horizontalReach * cos(coxaAngle);
    double y = horizontalReach * sin(coxaAngle);

    response->foot_position.x = x;
    response->foot_position.y = y;
    response->foot_position.z = z;
}
