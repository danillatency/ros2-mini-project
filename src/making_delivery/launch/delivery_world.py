import os

from launch import LaunchDescription
from launch.actions import ExecuteProcess, SetEnvironmentVariable, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def name_of_tag(tag, xml_path):
    string_to_find = f"{tag} name=\""
    found = str()
    extracted_name = str()
    for character in open(xml_path, 'r', encoding="utf-8").read():
        if found == string_to_find:
            if character == '"':
                return extracted_name
            else:
                extracted_name += character
            continue
        if string_to_find[len(found)] == character:
            found += character
        else:
            found = str()


def generate_launch_description():
    package_path = get_package_share_directory("making_delivery")
    world_path = os.path.join(package_path, "worlds", "delivery_world.sdf")
    robot_template_path = os.path.join(package_path, "robots", "deliverer_template.sdf")
    rviz_config_path = os.path.join(package_path, "rviz", "config.rviz")
    rviz_model_path = os.path.join(package_path, "rviz", "deliverer_0.sdf")
    world_name = name_of_tag("world", world_path)

    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{
            "robot_description": open(rviz_model_path, 'r', encoding="utf-8").read()
        }]
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", rviz_config_path]
    )

    gazebo_world_launcher = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory("ros_gz_sim"), "launch", "gz_sim.launch.py")
        ),
        launch_arguments={"gz_args": world_path}.items()
    )

    delivery_composition_node = Node(
        package="making_delivery",
        executable="package_executor",
        parameters=[{
            "robot_template_path": robot_template_path,
            "world_name": world_name
        }]
    )

    ros_to_gazebo_forces_bridge_node = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[f"/world/{world_name}/wrench@ros_gz_interfaces/msg/EntityWrench]gz.msgs.EntityWrench"]
    )

    ros_to_gazebo_persistent_forces_bridge_node = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[f"/world/{world_name}/wrench/persistent@ros_gz_interfaces/msg/EntityWrench]gz.msgs.EntityWrench"]
    )

    ros_to_gazebo_clear_forces_bridge_node = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[f"/world/{world_name}/wrench/clear@ros_gz_interfaces/msg/Entity]gz.msgs.Entity"]
    )

    return LaunchDescription([
        SetEnvironmentVariable("GZ_SIM_RESOURCE_PATH", os.path.join(package_path, "textures")),
        robot_state_publisher_node,
        rviz_node,
        gazebo_world_launcher,
        delivery_composition_node,
        ros_to_gazebo_forces_bridge_node,
        ros_to_gazebo_persistent_forces_bridge_node,
        ros_to_gazebo_clear_forces_bridge_node
    ])
