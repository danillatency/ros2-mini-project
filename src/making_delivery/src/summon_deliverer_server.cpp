#include <rclcpp/rclcpp.hpp>
#include <making_delivery/srv/summon_deliverer.hpp>
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

std::string openAndRead(std::string filename) {
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    t.close();
    return buffer.str();
}


class SummonDeliverer final : public rclcpp::Node {
public:
    rclcpp::Service<making_delivery::srv::SummonDeliverer>::SharedPtr summonDeliverer;
    rclcpp::executors::MultiThreadedExecutor *executor;
    std::vector<std::shared_ptr<ros_gz_bridge::RosGzBridge>> delivererBridges;
    gz::transport::Node gazeboTransportNode;
    gz::msgs::EntityFactory gazeboTransportRequest;
    gz::msgs::Boolean gazeboTransportResponse;

    SummonDeliverer(rclcpp::executors::MultiThreadedExecutor *executor) : Node("summon_deliverer") {
        this->executor = executor;
        this->declare_parameter<std::string>("robot_template_path", "");
        this->declare_parameter<std::string>("world_name", "");
        this->summonDeliverer = this->create_service<making_delivery::srv::SummonDeliverer>(
            "/summon_deliverer", std::bind(&SummonDeliverer::summon, this,
                                           std::placeholders::_1, std::placeholders::_2));
    }

    void establishBridge(std::string direction, std::string rosMessageType,
                         std::string gazeboMessageType,
                         std::string bridgingTopic, std::string reliability, std::string durability) {
        this->delivererBridges.push_back(std::make_shared<ros_gz_bridge::RosGzBridge>(
            rclcpp::NodeOptions().parameter_overrides({
                rclcpp::Parameter("bridge_names", std::vector<std::string>({"ros_gz_bridge"})),
                rclcpp::Parameter("bridges.ros_gz_bridge.ros_type_name", rosMessageType),
                rclcpp::Parameter("bridges.ros_gz_bridge.ros_topic_name", bridgingTopic),
                rclcpp::Parameter("bridges.ros_gz_bridge.gz_type_name", gazeboMessageType),
                rclcpp::Parameter("bridges.ros_gz_bridge.gz_topic_name", bridgingTopic),
                rclcpp::Parameter("bridges.ros_gz_bridge.direction", direction),
                rclcpp::Parameter("qos_overrides." + bridgingTopic + ".publisher.reliability", reliability),
                rclcpp::Parameter("qos_overrides." + bridgingTopic + ".publisher.durability", durability),
                rclcpp::Parameter("qos_overrides." + bridgingTopic + ".publisher.history", "keep_last")
            })));
        this->executor->add_node(this->delivererBridges.back());
    }

    void summon(const making_delivery::srv::SummonDeliverer::Request::SharedPtr &request,
                const making_delivery::srv::SummonDeliverer::Response::SharedPtr &response) {
        try {
            std::string robotTemplate = openAndRead(this->get_parameter("robot_template_path").as_string());
            robotTemplate.replace(robotTemplate.find("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            robotTemplate.replace(robotTemplate.rfind("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            this->gazeboTransportRequest.set_sdf(robotTemplate);
            gz::math::Pose3d pose{
                request->position.x, request->position.y, request->position.z,
                request->rotation.x * M_PI / 180,
                request->rotation.y * M_PI / 180,
                request->rotation.z * M_PI / 180
            };
            gz::msgs::Set(this->gazeboTransportRequest.mutable_pose(), pose);
            bool result;
            while (rclcpp::ok()) {
                if (gazeboTransportNode.Request("/world/" + this->get_parameter("world_name").as_string() +
                                                "/create", this->gazeboTransportRequest, 5000,
                                                this->gazeboTransportResponse, result)) {
                    if (result && this->gazeboTransportResponse.data()) {
                        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Entity has been created!");
                    } else {
                        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Failure of entity creation");
                    }
                    break;
                }
                RCLCPP_WARN(rclcpp::get_logger("rclcpp"), "Waiting for gz create service...");
            }
            this->establishBridge("GZ_TO_ROS", "geometry_msgs/msg/TransformStamped", "gz.msgs.Pose",
                                  '/' + request->frame_id + "/pose", "best_effort", "volatile");
            this->establishBridge("GZ_TO_ROS", "sensor_msgs/msg/LaserScan", "gz.msgs.LaserScan",
                                  '/' + request->frame_id + "_lidar", "best_effort", "volatile");
            response->summoned = true;
        } catch (std::exception &e) {
            response->summoned = false;
            std::cout << e.what();
        }
    }
};


int main(const int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    auto executor = rclcpp::executors::MultiThreadedExecutor();
    const auto node = std::make_shared<SummonDeliverer>(&executor);
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
}
