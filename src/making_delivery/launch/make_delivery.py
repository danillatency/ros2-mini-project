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

    gazebo_node = Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        output="screen",
        arguments=["-file", model_sdf, "-entity", "deliverer", "-x", '0', "-y", '0', "-z", '0']
    )

    return LaunchDescription([
        SetEnvironmentVariable("GAZEBO_MODEL_PATH", models_path),
        ExecuteProcess(
            cmd=["gazebo", "--verbose", world_path, "-s", "libgazebo_ros_factory.so"],
            output="screen"
        ),
        gazebo_node
    ])
