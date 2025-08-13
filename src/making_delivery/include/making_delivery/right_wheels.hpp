#pragma once
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <ros_gz_interfaces/msg/entity_wrench.hpp>
#include <making_delivery/srv/start_right_wheels.hpp>

using namespace std::chrono_literals;


class RightWheels : public rclcpp::Node {
public:
    rclcpp::Publisher<ros_gz_interfaces::msg::EntityWrench>::SharedPtr persistentForcePublisher;
    rclcpp::Service<making_delivery::srv::StartRightWheels>::SharedPtr startServer;

    RightWheels() : Node("right_wheels") {
        this->declare_parameter("world_name", "empty_world");
        this->persistentForcePublisher = this->create_publisher<ros_gz_interfaces::msg::EntityWrench>(
            "/world/" + this->get_parameter("world_name").as_string() + "/wrench/persistent",
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
        this->startServer = this->create_service<making_delivery::srv::StartRightWheels>(
            "/start_right_wheels", std::bind(&RightWheels::start, this, std::placeholders::_1, std::placeholders::_2),
            rclcpp::QoS(rclcpp::KeepLast(10)).reliable().durability_volatile());
    }

    void start(const making_delivery::srv::StartRightWheels::Request::SharedPtr request,
        const making_delivery::srv::StartRightWheels::Response::SharedPtr response) {
        auto message = ros_gz_interfaces::msg::EntityWrench();
        message.entity.type = ros_gz_interfaces::msg::Entity().LINK;
        message.wrench.force.x = 0;
        message.wrench.force.y = 0;
        message.wrench.force.z = 0;
        message.wrench.torque.x = 0;
        message.wrench.torque.y = 10;
        message.wrench.torque.z = 0;
        message.entity.name = request->frame_id + "::right_front_wheel_link";
        this->persistentForcePublisher->publish(message);
        message.entity.name = request->frame_id + "::right_middle_wheel_link";
        this->persistentForcePublisher->publish(message);
        message.entity.name = request->frame_id + "::right_back_wheel_link";
        this->persistentForcePublisher->publish(message);
        response->started = true;
    }
};
