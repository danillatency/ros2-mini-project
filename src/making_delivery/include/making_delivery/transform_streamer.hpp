#pragma once
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <making_delivery/srv/subscribe_transform_streamer.hpp>
#include <tf2_ros/transform_broadcaster.h>

class TransformStreamer final : public rclcpp::Node {
public:
    std::vector<rclcpp::Subscription<geometry_msgs::msg::TransformStamped>::SharedPtr> subscribers;
    rclcpp::Service<making_delivery::srv::SubscribeTransformStreamer>::SharedPtr addSubscription;
    std::shared_ptr<tf2_ros::TransformBroadcaster> dynamicBroadcaster;

    TransformStreamer() : Node("transform_streamer") {
        this->addSubscription = this->create_service<making_delivery::srv::SubscribeTransformStreamer>(
            "/subscribe_transform_streamer", std::bind(&TransformStreamer::subscribe, this,
                                                       std::placeholders::_1, std::placeholders::_2));
        this->dynamicBroadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(this);
    }

    void subscribe(const making_delivery::srv::SubscribeTransformStreamer::Request::SharedPtr &request,
                   const making_delivery::srv::SubscribeTransformStreamer::Response::SharedPtr &response) {
        this->subscribers.push_back(this->create_subscription<geometry_msgs::msg::TransformStamped>(
            request->topic_name, rclcpp::QoS(rclcpp::KeepLast(10)).best_effort().durability_volatile(),
            std::bind(&TransformStreamer::stream, this, std::placeholders::_1)));
        response->subscribed = true;
    }

    void stream(const geometry_msgs::msg::TransformStamped::SharedPtr message) {
        this->dynamicBroadcaster->sendTransform(*message);
    }
};
