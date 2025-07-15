#include "rclcpp/rclcpp.hpp"
#include <geometry_msgs/msg/quaternion.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <chrono>

using namespace std::chrono_literals;


class LeftFrontWheel : public rclcpp::Node {
public:
    rclcpp::TimerBase::SharedPtr timer;
    std::shared_ptr<tf2_ros::TransformBroadcaster> dynamicBroadcaster;
    double currentRadians = 0;

    LeftFrontWheel() : Node("left_front_wheel") {
        this->timer = this->create_wall_timer(50ms, std::bind(&LeftFrontWheel::rotate, this));
        this->dynamicBroadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(this);
    }

    void rotate() {
        auto message = geometry_msgs::msg::TransformStamped();
        message.header.stamp = this->get_clock()->now();
        message.header.frame_id = "base_link";
        message.child_frame_id = "left_front_wheel_link";
        message.transform.translation.x = 10;
        message.transform.translation.y = 10;
        message.transform.translation.z = 5;
        tf2::Quaternion quaternion;
        quaternion.setRPY(1.57, this->currentRadians, 0);
        message.transform.rotation.x = quaternion.x();
        message.transform.rotation.y = quaternion.y();
        message.transform.rotation.z = quaternion.z();
        message.transform.rotation.w = quaternion.w();
        this->dynamicBroadcaster->sendTransform(message);
        this->currentRadians += 0.05;
    }
};


int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<LeftFrontWheel>();
    rclcpp::spin(node);
    rclcpp::shutdown();
}
