#include <rclcpp/rclcpp.hpp>
#include <making_delivery/srv/summon_deliverer.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <cmath>
#include <ros_gz_bridge/ros_gz_bridge.hpp>

using namespace std::chrono_literals;

std::string openAndRead(std::string filename) {
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    t.close();
    return buffer.str();
}

void cmd(const std::string *arguments) {
    std::string command = "";
    for (const std::string *i = &arguments[0];; i++)
        try {
            const std::string current = *i;
            command += current + ' ';
        } catch (...) {
            break;
        }
    system(command.c_str());
}

class TestPublisher final : public rclcpp::Node {
public:
    rclcpp::Publisher<geometry_msgs::msg::Point>::SharedPtr publisher;
    rclcpp::TimerBase::SharedPtr timer;

    TestPublisher() : Node("test_publisher") {
        this->publisher = this->create_publisher<geometry_msgs::msg::Point>("/geometry_dash", 10);
        this->timer = this->create_wall_timer(500ms, std::bind(&TestPublisher::timeout, this));
    }

    void timeout() const {
        const auto message = geometry_msgs::msg::Point();
        this->publisher->publish(message);
    }
};

class SummonDeliverer final : public rclcpp::Node {
public:
    rclcpp::Service<making_delivery::srv::SummonDeliverer>::SharedPtr summonDeliverer;
    rclcpp::executors::MultiThreadedExecutor *executor;
    std::shared_ptr<ros_gz_bridge::RosGzBridge> *testPublishers;


    SummonDeliverer(rclcpp::executors::MultiThreadedExecutor *executor) : Node("summon_deliverer") {
        this->executor = executor;
        this->declare_parameter<std::string>("robot_template_path", "");
        this->declare_parameter<std::string>("world_name", "");
        this->declare_parameter<std::string>("pose_from_gazebo", "");
        this->summonDeliverer = this->create_service<making_delivery::srv::SummonDeliverer>(
            "/summon_deliverer", std::bind(&SummonDeliverer::summon, this,
                                           std::placeholders::_1, std::placeholders::_2));
        this->testPublishers = new std::shared_ptr<ros_gz_bridge::RosGzBridge>[1];
    }

    void summon(const making_delivery::srv::SummonDeliverer::Request::SharedPtr &request,
                const making_delivery::srv::SummonDeliverer::Response::SharedPtr &response) {
        try {
            std::string robotTemplate = openAndRead(this->get_parameter("robot_template_path").as_string());
            robotTemplate.replace(robotTemplate.find("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            robotTemplate.replace(robotTemplate.rfind("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            const std::string spawn_arguments[] = {
                "ros2", "run", "ros_gz_sim", "create", "-world",
                this->get_parameter("world_name").as_string(),
                "-string",
                "'" + robotTemplate + "'",
                "-x",
                std::to_string(request->position.x),
                "-y",
                std::to_string(request->position.y),
                "-z",
                std::to_string(request->position.z),
                "-R",
                std::to_string(request->rotation.x * M_PI / 180),
                "-P",
                std::to_string(request->rotation.y * M_PI / 180),
                "-Y",
                std::to_string(request->rotation.z * M_PI / 180),
            };
            cmd(spawn_arguments);
            auto pose_bridge_node = std::make_shared<ros_gz_bridge::RosGzBridge>(
                rclcpp::NodeOptions().parameter_overrides({
                    rclcpp::Parameter("bridge_names", std::vector<std::string>({"ros_gz_bridge"})),
                    rclcpp::Parameter("bridges.ros_gz_bridge.ros_type_name", "geometry_msgs/msg/TransformStamped"),
                    rclcpp::Parameter("bridges.ros_gz_bridge.ros_topic_name", "/deliverer_0/pose"),
                    rclcpp::Parameter("bridges.ros_gz_bridge.gz_type_name", "gz.msgs.Pose"),
                    rclcpp::Parameter("bridges.ros_gz_bridge.gz_topic_name", "/deliverer_0/pose"),
                    rclcpp::Parameter("bridges.ros_gz_bridge.direction", "GZ_TO_ROS"),
                    rclcpp::Parameter("qos_overrides./deliverer_0/pose.publisher.reliability", "best_effort")
                }));
            this->executor->add_node(pose_bridge_node);
            this->testPublishers[0] = pose_bridge_node;
            /*std::string robotPoseTemplate = openAndRead(this->get_parameter("pose_from_gazebo").as_string());
            robotPoseTemplate.replace(robotPoseTemplate.find("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            robotPoseTemplate.replace(robotPoseTemplate.rfind("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            RCLCPP_INFO(rclcpp::get_logger("rclcpp"), ("\"bridges:=" + robotPoseTemplate + "\"").c_str());
            const std::string pose_bridge_arguments[] = {
                "ros2", "run", "ros_gz_bridge", "parameter_bridge",
                "--ros-args", "-p",
                "bridges:=\"" + robotPoseTemplate + "\"",
                "&"
            };
            cmd(pose_bridge_arguments);
            const std::string lidar_bridge_arguments[] = {
                "ros2 run ros_gz_bridge parameter_bridge",
                '/' + request->frame_id + "_lidar@sensor_msgs/msg/LaserScan[gz.msgs.LaserScan",
                "&"
            };
            cmd(lidar_bridge_arguments);*/
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
