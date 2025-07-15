import os

from launch import LaunchDescription
from launch.actions import ExecuteProcess, SetEnvironmentVariable
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    package_path = get_package_share_directory("making_delivery")
    world_path = os.path.join(package_path, "worlds", "empty_world.world")
    models_path = os.path.join(package_path, "models")
    model_sdf = os.path.join(models_path, "deliverer", "model.sdf")
    rviz_config = os.path.join(package_path, "rviz", "config.rviz")

    gazebo_node = Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        output="screen",
        arguments=["-file", model_sdf, "-entity", "deliverer", "-x", '0', "-y", '0', "-z", '0']
    )

    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        parameters=[{
            "robot_description": open(model_sdf, 'r', encoding="utf-8").read()
        }]
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        output="screen",
        arguments=["-d", rviz_config]
    )

    left_front_wheel_rotating = Node(
        package="making_delivery",
        executable="left_front_wheel",
        output="screen"
    )

    return LaunchDescription([
        robot_state_publisher_node,
        left_front_wheel_rotating,
        rviz_node,
        SetEnvironmentVariable("GAZEBO_MODEL_PATH", models_path),
        ExecuteProcess(
            cmd=["gazebo", "--verbose", world_path, "-s", "libgazebo_ros_factory.so",
                 "-s", "libgazebo_ros_api_plugin.so",
                 "-s", "libgazebo_ros_paths_plugin.so"],
            output="screen"
        ),
        gazebo_node
    ])
