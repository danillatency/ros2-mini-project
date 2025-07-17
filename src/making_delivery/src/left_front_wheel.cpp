#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <ros_gz_interfaces/msg/entity_wrench.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <chrono>

using namespace std::chrono_literals;


class LeftFrontWheel : public rclcpp::Node {
public:
    rclcpp::TimerBase::SharedPtr timer;
    rclcpp::TimerBase::SharedPtr timer2;
    std::shared_ptr<tf2_ros::TransformBroadcaster> dynamicBroadcaster;
    rclcpp::Publisher<ros_gz_interfaces::msg::EntityWrench>::SharedPtr publisher;
    std::unique_ptr<tf2_ros::Buffer> tfBuffer;
    std::shared_ptr<tf2_ros::TransformListener> tfListener;

    double currentRadians = 0;

    LeftFrontWheel() : Node("left_front_wheel") {
        this->timer = this->create_wall_timer(50ms, std::bind(&LeftFrontWheel::rotate, this));
        this->timer2 = this->create_wall_timer(1000ms, std::bind(&LeftFrontWheel::r, this));
        this->dynamicBroadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(this);
        this->publisher = this->create_publisher<ros_gz_interfaces::msg::EntityWrench>("/world/empty_world/wrench", 10);
        this->tfBuffer = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        this->tfListener = std::make_shared<tf2_ros::TransformListener>(*this->tfBuffer);
    }

    void r() {
        geometry_msgs::msg::TransformStamped m;
        m = this->tfBuffer->lookupTransform("left_front_wheel_link", "base_link", tf2::TimePointZero);
        RCLCPP_INFO(this->get_logger(), std::to_string(m.transform.rotation.z).c_str());


        auto message = ros_gz_interfaces::msg::EntityWrench();
        message.entity.name = "deliverer::left_front_wheel_link";
        message.entity.type = ros_gz_interfaces::msg::Entity().LINK;
        message.wrench.force.x = 0;
        message.wrench.force.y = 0;
        message.wrench.force.z = 0;
        message.wrench.torque.x = 0;
        message.wrench.torque.y = 0;
        message.wrench.torque.z = 1000000;
        this->publisher->publish(message);
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
