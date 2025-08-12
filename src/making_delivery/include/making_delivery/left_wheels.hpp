#pragma once
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <ros_gz_interfaces/msg/entity_wrench.hpp>

using namespace std::chrono_literals;


class LeftWheels : public rclcpp::Node {
public:
    bool torque = true;
    // rclcpp::TimerBase::SharedPtr timerAddTorque;
    std::unique_ptr<tf2_ros::Buffer> tfBuffer;
    std::shared_ptr<tf2_ros::TransformListener> tfListener;
    rclcpp::Publisher<ros_gz_interfaces::msg::EntityWrench>::SharedPtr persistentForcePublisher;

    LeftWheels() : Node("left_wheels") {
        // this->timerAddTorque = this->create_wall_timer(50ms, std::bind(&LeftWheels::addTorque, this));
        this->tfBuffer = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        this->tfListener = std::make_shared<tf2_ros::TransformListener>(*this->tfBuffer);
        this->declare_parameter("world_name", "empty_world");
        this->persistentForcePublisher = this->create_publisher<ros_gz_interfaces::msg::EntityWrench>(
            "/world/" + this->get_parameter("world_name").as_string() + "/wrench/persistent",
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
    }

    void addTorque() {
        geometry_msgs::msg::TransformStamped leftWheelTransformation;
        leftWheelTransformation = this->tfBuffer->lookupTransform("base_link", "left_front_wheel",  tf2::TimePointZero);
        RCLCPP_INFO(this->get_logger(), std::to_string(leftWheelTransformation.transform.rotation.z).c_str());
    }
};
