#include <rclcpp/rclcpp.hpp>
#include <chrono>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/pose_array.hpp>

using namespace std::chrono_literals;


class LeftWheels : public rclcpp::Node {
public:
    rclcpp::TimerBase::SharedPtr timerAddTorque;
    std::unique_ptr<tf2_ros::Buffer> tfBuffer;
    std::shared_ptr<tf2_ros::TransformListener> tfListener;
    rclcpp::Subscription<geometry_msgs::msg::PoseArray>::SharedPtr subscriber;

    LeftWheels() : Node("left_wheels") {
        this->timerAddTorque = this->create_wall_timer(50ms, std::bind(&LeftWheels::addTorque, this));
        this->tfBuffer = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        this->tfListener = std::make_shared<tf2_ros::TransformListener>(*this->tfBuffer);
        this->subscriber = this->create_subscription<geometry_msgs::msg::PoseArray>("/world/empty_world/pose/info", 10,
            std::bind(&LeftWheels::poseArrayToTf, this, std::placeholders::_1));
    }

    void poseArrayToTf(const geometry_msgs::msg::PoseArray::SharedPtr message) {
    }

    void addTorque() {
        geometry_msgs::msg::TransformStamped leftWheelTransformation;
        //leftWheelTransformation = this->tfBuffer->lookupTransform("base_link", "left_front_wheel",  tf2::TimePointZero);
        RCLCPP_INFO(this->get_logger(), std::to_string(leftWheelTransformation.transform.rotation.z).c_str());
    }
};


int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<LeftWheels>();
    rclcpp::spin(node);
    rclcpp::shutdown();
}
