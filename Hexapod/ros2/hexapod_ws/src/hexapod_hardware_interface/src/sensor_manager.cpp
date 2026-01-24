#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "hexapod_interfaces/msg/leg_contact.hpp"
#include "hexapod_interfaces/msg/power_status.hpp"

using Imu = sensor_msgs::msg::Imu;
using LegContact = hexapod_interfaces::msg::LegContact;
using PowerStatus = hexapod_interfaces::msg::PowerStatus;

class SensorManager : public rclcpp::Node{public: SensorManager() : Node("sensor_manager"){
    imu_publisher = this->create_publisher<Imu>("imu_data", 10);
    leg_contact_publisher = this->create_publisher<LegContact>("leg_contact_data", 10);
    power_status_publisher = this->create_publisher<PowerStatus>("power_status_data", 10);
}    
private:
    rclcpp::Publisher<Imu>::SharedPtr imu_publisher;
    rclcpp::Publisher<LegContact>::SharedPtr leg_contact_publisher;
    rclcpp::Publisher<PowerStatus>::SharedPtr power_status_publisher;
};

int main(int argc, char **argv){    
    rclcpp::init(argc, argv);    
    auto node = std::make_shared<SensorManager>();    
    rclcpp::spin(node);    
    rclcpp::shutdown();    
    return 0;
}