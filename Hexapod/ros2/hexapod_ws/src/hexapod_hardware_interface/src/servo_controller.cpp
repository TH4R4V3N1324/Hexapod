#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "std_srvs/srv/set_bool.hpp"

using JointState = sensor_msgs::msg::JointState;
using std_srvs::srv::SetBool;
using std::placeholders::_1;
using std::placeholders::_2;

class ServoController : public rclcpp::Node{public: ServoController() : Node("servo_controller"){
    joint_command_subscriber = this->create_subscription<JointState>("joint_commands", 10, std::bind(&ServoController::handle_joint_command, this, _1));
    joint_state_publisher = this->create_publisher<JointState>("joint_states", 10);
    emergency_stop_service = this->create_service<std_srvs::srv::SetBool>("emergency_stop", std::bind(&ServoController::handle_emergency_stop, this, _1, _2));
    timer = this->create_wall_timer(std::chrono::milliseconds(100), std::bind(&ServoController::publish_joint_state, this));
    joint_names = {
        "coxa_1", "femur_1", "tibia_1",
        "coxa_2", "femur_2", "tibia_2",
        "coxa_3", "femur_3", "tibia_3",
        "coxa_4", "femur_4", "tibia_4",
        "coxa_5", "femur_5", "tibia_5",
        "coxa_6", "femur_6", "tibia_6"
    };
    joint_positions.resize(joint_names.size(), 0.0);
}    
private:
    rclcpp::Subscription<JointState>::SharedPtr joint_command_subscriber;
    rclcpp::Publisher<JointState>::SharedPtr joint_state_publisher;
    rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr emergency_stop_service;
    rclcpp::TimerBase::SharedPtr timer;
    std::vector<std::string> joint_names;
    std::vector<double> joint_positions;
public:
    void handle_joint_command(const JointState::SharedPtr msg){
        // Process incoming joint command and control servos
        RCLCPP_INFO(this->get_logger(), "Received joint command with %zu positions", msg->position.size());
        // Publish current joint states as a response (mocked here)
        auto joint_state_msg = JointState();
        joint_state_msg.name = msg->name;
        joint_state_msg.position = msg->position; // In real implementation, this would be the actual servo positions
        joint_state_publisher->publish(joint_state_msg);
    }

    void handle_emergency_stop(const std::shared_ptr<SetBool::Request> request,
                               std::shared_ptr<SetBool::Response> response){
        if(request->data){
            // Activate emergency stop
            RCLCPP_WARN(this->get_logger(), "Emergency stop activated!");
            response->success = true;
            response->message = "Emergency stop activated.";
        } else {
            // Deactivate emergency stop
            RCLCPP_INFO(this->get_logger(), "Emergency stop deactivated.");
            response->success = true;
            response->message = "Emergency stop deactivated.";
        }
    }

    void publish_joint_state(){
        // Periodically publish joint states (mocked here)
        auto joint_state_msg = JointState();
        joint_state_msg.header.stamp = this->now();
        joint_state_msg.name = joint_names;
        joint_state_msg.position = joint_positions;
        joint_state_publisher->publish(joint_state_msg);
        RCLCPP_DEBUG(this->get_logger(), "Published joint states.");
    }
};

int main(int argc, char **argv){    
    rclcpp::init(argc, argv);    
    auto node = std::make_shared<ServoController>();    
    rclcpp::spin(node);    
    rclcpp::shutdown();    
    return 0;
}