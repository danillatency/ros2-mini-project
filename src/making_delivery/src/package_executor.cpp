#include <rclcpp/rclcpp.hpp>
#include <making_delivery/summon_deliverer_server.hpp>

int main(const int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    auto executor = rclcpp::executors::MultiThreadedExecutor();
    const auto node = std::make_shared<SummonDeliverer>(&executor);
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
}
