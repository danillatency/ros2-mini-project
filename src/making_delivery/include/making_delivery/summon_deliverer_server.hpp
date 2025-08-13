#pragma once
#include <rclcpp/rclcpp.hpp>
#include <making_delivery/srv/summon_deliverer.hpp>
#include <making_delivery/srv/subscribe_transform_streamer.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <cmath>
#include <ros_gz_bridge/ros_gz_bridge.hpp>
#include <gz/transport/Node.hh>
#include <gz/msgs/entity_factory.pb.h>
#include <gz/math/Pose3.hh>
#include <gz/msgs.hh>

using namespace std::chrono_literals;

inline std::string openAndRead(const std::string &filename) {
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    t.close();
    return buffer.str();
}


class SummonDeliverer final : public rclcpp::Node {
public:
    rclcpp::Service<making_delivery::srv::SummonDeliverer>::SharedPtr summonDelivererServer;
    rclcpp::Client<making_delivery::srv::SubscribeTransformStreamer>::SharedPtr subscribeStreamerOnPoseClient;
    rclcpp::executors::MultiThreadedExecutor *executor;
    std::vector<std::shared_ptr<ros_gz_bridge::RosGzBridge> > delivererBridges;
    gz::transport::Node gazeboTransportNode;

    SummonDeliverer(rclcpp::executors::MultiThreadedExecutor *executor) : Node("summon_deliverer") {
        this->executor = executor;
        this->declare_parameter<std::string>("robot_template_path", "");
        this->declare_parameter<std::string>("world_name", "");
        this->summonDelivererServer = this->create_service<making_delivery::srv::SummonDeliverer>(
            "/summon_deliverer", std::bind(&SummonDeliverer::summon, this,
                                           std::placeholders::_1, std::placeholders::_2));
        this->subscribeStreamerOnPoseClient = this->create_client<making_delivery::srv::SubscribeTransformStreamer>(
            "/subscribe_transform_streamer");
    }

    void establishBridge(const std::string &direction, const std::string &rosMessageType,
                         const std::string &gazeboMessageType, const std::string &bridgingTopic,
                         const std::string &reliability, const std::string &durability) {
        std::string bridgeName = "ros_gz_bridge_" + std::to_string(this->delivererBridges.size());
        this->delivererBridges.push_back(std::make_shared<ros_gz_bridge::RosGzBridge>(
            rclcpp::NodeOptions().parameter_overrides({
                rclcpp::Parameter("bridge_names", std::vector({bridgeName})),
                rclcpp::Parameter("bridges." + bridgeName + ".ros_type_name", rosMessageType),
                rclcpp::Parameter("bridges." + bridgeName + ".ros_topic_name", bridgingTopic),
                rclcpp::Parameter("bridges." + bridgeName + ".gz_type_name", gazeboMessageType),
                rclcpp::Parameter("bridges." + bridgeName + ".gz_topic_name", bridgingTopic),
                rclcpp::Parameter("bridges." + bridgeName + ".direction", direction),
                rclcpp::Parameter("qos_overrides." + bridgingTopic + ".publisher.reliability", reliability),
                rclcpp::Parameter("qos_overrides." + bridgingTopic + ".publisher.durability", durability),
                rclcpp::Parameter("qos_overrides." + bridgingTopic + ".publisher.history", "keep_last")
            }).arguments({
                "--ros-args", "-r", "__node:=" + bridgeName
            })));
        this->executor->add_node(this->delivererBridges.back());
    }

    void summon(const making_delivery::srv::SummonDeliverer::Request::SharedPtr &request,
                const making_delivery::srv::SummonDeliverer::Response::SharedPtr &response) {
        try {
            std::string robotTemplate = openAndRead(this->get_parameter("robot_template_path").as_string());
            robotTemplate.replace(robotTemplate.find("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            robotTemplate.replace(robotTemplate.rfind("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            gz::msgs::EntityFactory gazeboTransportRequest;
            gz::msgs::Boolean gazeboTransportResult;
            gazeboTransportRequest.set_sdf(robotTemplate);
            gz::math::Pose3d pose{
                request->position.x, request->position.y, request->position.z,
                request->rotation.x * M_PI / 180,
                request->rotation.y * M_PI / 180,
                request->rotation.z * M_PI / 180
            };
            gz::msgs::Set(gazeboTransportRequest.mutable_pose(), pose);
            bool result;
            while (rclcpp::ok()) {
                if (gazeboTransportNode.Request("/world/" + this->get_parameter("world_name").as_string() +
                                                "/create", gazeboTransportRequest, 5000,
                                                gazeboTransportResult, result)) {
                    if (result && gazeboTransportResult.data()) {
                        RCLCPP_INFO(this->get_logger(), "Entity has been created!");
                    } else {
                        RCLCPP_INFO(this->get_logger(), "Failure of entity creation");
                    }
                    break;
                }
                RCLCPP_WARN(this->get_logger(), "Waiting for the gz create service");
            }
            this->establishBridge("GZ_TO_ROS", "geometry_msgs/msg/TransformStamped", "gz.msgs.Pose",
                                  '/' + request->frame_id + "/pose", "best_effort", "volatile");
            this->establishBridge("GZ_TO_ROS", "sensor_msgs/msg/LaserScan", "gz.msgs.LaserScan",
                                  '/' + request->frame_id + "_lidar", "best_effort", "volatile");
            const auto subscribeRequest = std::make_shared<making_delivery::srv::SubscribeTransformStreamer::Request>();
            subscribeRequest->topic_name = '/' + request->frame_id + "/pose";
            while (!this->subscribeStreamerOnPoseClient->wait_for_service(1s)) {
                if (!rclcpp::ok()) {
                    RCLCPP_ERROR(this->get_logger(), "Interrupted waiting for the transform subscribe server");
                    throw std::exception();
                }
                RCLCPP_WARN(this->get_logger(), "Waiting for the transform subscribe service");
            }
            auto subscribeResult = this->subscribeStreamerOnPoseClient->async_send_request(
                subscribeRequest,
                [this](rclcpp::Client<making_delivery::srv::SubscribeTransformStreamer>::SharedFuture result) {
                    if (result.get()->subscribed)
                        RCLCPP_INFO(this->get_logger(), "Entity has been subscribed!");
                    else
                        RCLCPP_INFO(this->get_logger(), "Failure of entity subscription");
                });
            response->summoned = true;
        } catch (std::exception &e) {
            response->summoned = false;
            RCLCPP_ERROR(this->get_logger(), e.what());
        }
    }
};
