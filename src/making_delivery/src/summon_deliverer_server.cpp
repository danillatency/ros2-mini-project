#include <rclcpp/rclcpp.hpp>
#include <making_delivery/srv/summon_deliverer.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <cmath>

std::string openAndRead(std::string filename) {
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

void cmd(const std::string *arguments) {
    std::string command = "";
    for (const std::string* i = &arguments[0];; i++)
        try {
            const std::string current = *i;
            command += current + ' ';
        } catch (...) {
            break;
        }
    system(command.c_str());
}

class SummonDeliverer final : public rclcpp::Node {
public:
    rclcpp::Service<making_delivery::srv::SummonDeliverer>::SharedPtr summonDeliverer;

    SummonDeliverer() : Node("summon_deliverer") {
        this->declare_parameter<std::string>("robot_template_path", "");
        this->declare_parameter<std::string>("world_name", "");
        this->summonDeliverer = this->create_service<making_delivery::srv::SummonDeliverer>(
            "/summon_deliverer", std::bind(&SummonDeliverer::summon, this,
                                           std::placeholders::_1, std::placeholders::_2));
    }

    void summon(const making_delivery::srv::SummonDeliverer::Request::SharedPtr &request,
                const making_delivery::srv::SummonDeliverer::Response::SharedPtr &response) const {
        try {
            std::string robotTemplate = openAndRead(this->get_parameter("robot_template_path").as_string());
            robotTemplate.replace(robotTemplate.find("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            robotTemplate.replace(robotTemplate.rfind("{{ DELIVERER_NAME }}"), 20, request->frame_id);
            const std::string spawn_arguments[] = {
                "ros2 run ros_gz_sim create -world",
                this->get_parameter("world_name").as_string(),
                "-string",
                "'",
                robotTemplate,
                "'",
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
            const std::string bridge_arguments[] = {
                "ros2 run ros_gz_bridge parameter_bridge",
                '/' + request->frame_id + "/pose@geometry_msgs/msg/TransformStamped[gz.msgs.Pose",
                "&"
            };
            cmd(bridge_arguments);
            response->summoned = true;
        } catch (std::exception& e) {
            response->summoned = false;
            std::cout << e.what();
        }
    }
};


int main(const int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    const auto node = std::make_shared<SummonDeliverer>();
    rclcpp::spin(node);
    rclcpp::shutdown();
}
