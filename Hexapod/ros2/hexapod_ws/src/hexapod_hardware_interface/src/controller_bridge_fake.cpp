#include "rclcpp/rclcpp.hpp"
#include "hexapod_interfaces/msg/controller_raw.hpp"

using ControllerRaw = hexapod_interfaces::msg::ControllerRaw;

class ControllerBridge : public rclcpp::Node{public: ControllerBridge() : Node("controller_bridge_fake"){
    controller_data_publisher = this->create_publisher<ControllerRaw>("controller_raw_data", 10);
    timer = this->create_wall_timer(std::chrono::milliseconds(100), std::bind(&ControllerBridge::publish_controller_data, this));
}    
private:
    rclcpp::Publisher<ControllerRaw>::SharedPtr controller_data_publisher;
    rclcpp::TimerBase::SharedPtr timer;
public:
    void publish_controller_data(){
        auto controller_msg = ControllerRaw();
        // Populate controller_msg with data
        controller_msg.stamp = this->now();
        controller_data_publisher->publish(controller_msg);
    }
};

int main(int argc, char **argv){    
    rclcpp::init(argc, argv);    
    auto node = std::make_shared<ControllerBridge>();    
    rclcpp::spin(node);    
    rclcpp::shutdown();    
    return 0;
}