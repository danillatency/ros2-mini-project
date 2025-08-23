#pragma once
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <ros_gz_interfaces/msg/entity_wrench.hpp>
#include <making_delivery/srv/torque_left_wheels.hpp>

using namespace std::chrono_literals;


class LeftWheels : public rclcpp::Node {
public:
    // rclcpp::TimerBase::SharedPtr timerAddTorque;
    std::unique_ptr<tf2_ros::Buffer> tfBuffer;
    std::shared_ptr<tf2_ros::TransformListener> tfListener;
    rclcpp::Publisher<ros_gz_interfaces::msg::EntityWrench>::SharedPtr persistentForcePublisher;
    rclcpp::Publisher<ros_gz_interfaces::msg::Entity>::SharedPtr forceCleanerPublisher;
    rclcpp::Service<making_delivery::srv::TorqueLeftWheels>::SharedPtr torqueServer;

    LeftWheels() : Node("left_wheels") {
        // this->timerAddTorque = this->create_wall_timer(50ms, std::bind(&LeftWheels::addTorque, this));
        this->tfBuffer = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        this->tfListener = std::make_shared<tf2_ros::TransformListener>(*this->tfBuffer);
        this->declare_parameter("world_name", "");
        this->persistentForcePublisher = this->create_publisher<ros_gz_interfaces::msg::EntityWrench>(
            "/world/" + this->get_parameter("world_name").as_string() + "/wrench/persistent",
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
        this->forceCleanerPublisher = this->create_publisher<ros_gz_interfaces::msg::Entity>(
            "/world/" + this->get_parameter("world_name").as_string() + "/wrench/clear",
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
        this->torqueServer = this->create_service<making_delivery::srv::TorqueLeftWheels>(
            "/torque_left_wheels", std::bind(&LeftWheels::torque, this, std::placeholders::_1, std::placeholders::_2),
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
    }

    void addTorque() {
        geometry_msgs::msg::TransformStamped leftWheelTransformation;
        leftWheelTransformation = this->tfBuffer->lookupTransform("base_link", "left_front_wheel", tf2::TimePointZero);
        RCLCPP_INFO(this->get_logger(), std::to_string(leftWheelTransformation.transform.rotation.z).c_str());
    }

    void torque(const making_delivery::srv::TorqueLeftWheels::Request::SharedPtr request,
        const making_delivery::srv::TorqueLeftWheels::Response::SharedPtr response) {
        auto clearMessage = ros_gz_interfaces::msg::Entity();
        clearMessage.type = ros_gz_interfaces::msg::Entity().LINK;
        clearMessage.name = request->frame_id + "::left_front_wheel_link";
        this->forceCleanerPublisher->publish(clearMessage);
        clearMessage.name = request->frame_id + "::left_middle_wheel_link";
        this->forceCleanerPublisher->publish(clearMessage);
        clearMessage.name = request->frame_id + "::left_back_wheel_link";
        this->forceCleanerPublisher->publish(clearMessage);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        auto message = ros_gz_interfaces::msg::EntityWrench();
        message.entity.type = ros_gz_interfaces::msg::Entity().LINK;
        message.wrench.force.x = 0;
        message.wrench.force.y = 0;
        message.wrench.force.z = 0;
        message.wrench.torque.x = 0;
        message.wrench.torque.y = 0;
        message.wrench.torque.z = request->torque;
        message.entity.name = request->frame_id + "::left_front_wheel_link";
        this->persistentForcePublisher->publish(message);
        message.entity.name = request->frame_id + "::left_middle_wheel_link";
        this->persistentForcePublisher->publish(message);
        message.entity.name = request->frame_id + "::left_back_wheel_link";
        this->persistentForcePublisher->publish(message);

        response->applied = true;
    }
};
