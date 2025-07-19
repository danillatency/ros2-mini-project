#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <ros_gz_interfaces/msg/entity_wrench.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <chrono>

using namespace std::chrono_literals;


class Lid : public rclcpp::Node {
public:
    rclcpp::TimerBase::SharedPtr timer2;
    std::shared_ptr<tf2_ros::TransformBroadcaster> dynamicBroadcaster;
    rclcpp::Publisher<ros_gz_interfaces::msg::EntityWrench>::SharedPtr publisher;
    std::unique_ptr<tf2_ros::Buffer> tfBuffer;
    std::shared_ptr<tf2_ros::TransformListener> tfListener;
    bool direction = true;

    Lid() : Node("left_front_wheel") {
        this->timer2 = this->create_wall_timer(5000ms, std::bind(&Lid::r, this));
        // this->dynamicBroadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(this);
        this->publisher = this->create_publisher<ros_gz_interfaces::msg::EntityWrench>("/world/empty_world/wrench", 10);
        // this->tfBuffer = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        // this->tfListener = std::make_shared<tf2_ros::TransformListener>(*this->tfBuffer);
    }

    void r() {
        auto message = ros_gz_interfaces::msg::EntityWrench();
        message.entity.name = "deliverer::lid_link";
        message.entity.type = ros_gz_interfaces::msg::Entity().LINK;
        message.wrench.force.x = 0;
        message.wrench.force.y = 0;
        message.wrench.torque.x = 0;
        message.wrench.torque.z = 0;
        if (this->direction) {
//
            message.wrench.torque.y = 50;
        } else {
//
            message.wrench.torque.y = -50;
        }
        this->direction = !this->direction;
        this->publisher->publish(message);
        message.entity.name = "deliverer_1::lid_link";
        this->publisher->publish(message);
        message.entity.name = "deliverer_2::lid_link";
        this->publisher->publish(message);
    }
};


int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Lid>();
    rclcpp::spin(node);
    rclcpp::shutdown();
}
