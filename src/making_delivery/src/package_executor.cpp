#include <rclcpp/rclcpp.hpp>
#include <making_delivery/summon_deliverer_server.hpp>
#include <making_delivery/transform_streamer.hpp>
#include <making_delivery/left_wheels.hpp>

int main(const int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    auto executor = rclcpp::executors::MultiThreadedExecutor();
    const auto summonDelivererNode = std::make_shared<SummonDeliverer>(&executor);
    const auto transformStreamerNode = std::make_shared<TransformStreamer>();
    const auto leftWheelsNode = std::make_shared<LeftWheels>();
    executor.add_node(summonDelivererNode);
    executor.add_node(transformStreamerNode);
    executor.add_node(leftWheelsNode);
    executor.spin();
    rclcpp::shutdown();
}
