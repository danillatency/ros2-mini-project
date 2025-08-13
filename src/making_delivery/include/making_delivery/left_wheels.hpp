#pragma once
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <ros_gz_interfaces/msg/entity_wrench.hpp>
#include <making_delivery/srv/start_left_wheels.hpp>

using namespace std::chrono_literals;


class LeftWheels : public rclcpp::Node {
public:
    // rclcpp::TimerBase::SharedPtr timerAddTorque;
    std::unique_ptr<tf2_ros::Buffer> tfBuffer;
    std::shared_ptr<tf2_ros::TransformListener> tfListener;
    rclcpp::Publisher<ros_gz_interfaces::msg::EntityWrench>::SharedPtr persistentForcePublisher;
    rclcpp::Service<making_delivery::srv::StartLeftWheels>::SharedPtr startServer;

    LeftWheels() : Node("left_wheels") {
        // this->timerAddTorque = this->create_wall_timer(50ms, std::bind(&LeftWheels::addTorque, this));
        this->tfBuffer = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        this->tfListener = std::make_shared<tf2_ros::TransformListener>(*this->tfBuffer);
        this->declare_parameter("world_name", "empty_world");
        this->persistentForcePublisher = this->create_publisher<ros_gz_interfaces::msg::EntityWrench>(
            "/world/" + this->get_parameter("world_name").as_string() + "/wrench/persistent",
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
        this->startServer = this->create_service<making_delivery::srv::StartLeftWheels>(
            "/start_left_wheels", std::bind(&LeftWheels::start, this, std::placeholders::_1, std::placeholders::_2),
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
    }

    void addTorque() {
        geometry_msgs::msg::TransformStamped leftWheelTransformation;
        leftWheelTransformation = this->tfBuffer->lookupTransform("base_link", "left_front_wheel", tf2::TimePointZero);
        RCLCPP_INFO(this->get_logger(), std::to_string(leftWheelTransformation.transform.rotation.z).c_str());
    }

    void start(const making_delivery::srv::StartLeftWheels::Request::SharedPtr request,
        const making_delivery::srv::StartLeftWheels::Response::SharedPtr response) {
        auto message = ros_gz_interfaces::msg::EntityWrench();
        message.entity.type = ros_gz_interfaces::msg::Entity().LINK;
        message.wrench.force.x = 0;
        message.wrench.force.y = 0;
        message.wrench.force.z = 0;
        message.wrench.torque.x = 0;
        message.wrench.torque.y = 10;
        message.wrench.torque.z = 0;
        message.entity.name = request->frame_id + "::left_front_wheel_link";
        this->persistentForcePublisher->publish(message);
        message.entity.name = request->frame_id + "::left_middle_wheel_link";
        this->persistentForcePublisher->publish(message);
        message.entity.name = request->frame_id + "::left_back_wheel_link";
        this->persistentForcePublisher->publish(message);
        response->started = true;
    }
};
