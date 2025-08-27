#pragma once
#include <rclcpp/rclcpp.hpp>
#include <ros_gz_interfaces/msg/entity_wrench.hpp>
#include <making_delivery/srv/torque_right_wheels.hpp>

using namespace std::chrono_literals;


class RightWheels : public rclcpp::Node {
public:
    rclcpp::Publisher<ros_gz_interfaces::msg::EntityWrench>::SharedPtr persistentForcePublisher;
    rclcpp::Publisher<ros_gz_interfaces::msg::Entity>::SharedPtr forceCleanerPublisher;
    rclcpp::Service<making_delivery::srv::TorqueRightWheels>::SharedPtr torqueServer;

    RightWheels() : Node("right_wheels") {
        this->declare_parameter("world_name", "empty_world");
        this->persistentForcePublisher = this->create_publisher<ros_gz_interfaces::msg::EntityWrench>(
            "/world/" + this->get_parameter("world_name").as_string() + "/wrench/persistent",
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
        this->forceCleanerPublisher = this->create_publisher<ros_gz_interfaces::msg::Entity>(
            "/world/" + this->get_parameter("world_name").as_string() + "/wrench/clear",
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
        this->torqueServer = this->create_service<making_delivery::srv::TorqueRightWheels>(
            "/torque_right_wheels", std::bind(&RightWheels::torque, this, std::placeholders::_1, std::placeholders::_2),
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
    }

    void torque(const making_delivery::srv::TorqueRightWheels::Request::SharedPtr request,
        const making_delivery::srv::TorqueRightWheels::Response::SharedPtr response) const {
        auto clearMessage = ros_gz_interfaces::msg::Entity();
        clearMessage.type = ros_gz_interfaces::msg::Entity().LINK;
        clearMessage.name = request->frame_id + "::right_front_wheel_link";
        this->forceCleanerPublisher->publish(clearMessage);
        clearMessage.name = request->frame_id + "::right_middle_wheel_link";
        this->forceCleanerPublisher->publish(clearMessage);
        clearMessage.name = request->frame_id + "::right_back_wheel_link";
        this->forceCleanerPublisher->publish(clearMessage);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto message = ros_gz_interfaces::msg::EntityWrench();
        message.entity.type = ros_gz_interfaces::msg::Entity().LINK;
        message.wrench.force.x = 0;
        message.wrench.force.y = 0;
        message.wrench.force.z = 0;
        message.wrench.torque.x = 0;
        message.wrench.torque.y = request->torque;
        message.wrench.torque.z = 0;
        message.entity.name = request->frame_id + "::right_front_wheel_link";
        this->persistentForcePublisher->publish(message);
        message.entity.name = request->frame_id + "::right_middle_wheel_link";
        this->persistentForcePublisher->publish(message);
        message.entity.name = request->frame_id + "::right_back_wheel_link";
        this->persistentForcePublisher->publish(message);
        response->applied = true;
    }
};
