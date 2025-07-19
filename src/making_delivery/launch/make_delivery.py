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
    model_path = os.path.join(package_path, "robots", "deliverer.sdf")
    model_path1 = os.path.join(package_path, "robots", "deliverer_1.sdf")
    model_path2 = os.path.join(package_path, "robots", "deliverer_2.sdf")
    rviz_config_path = os.path.join(package_path, "rviz", "config.rviz")
    world_name = name_of_tag("world", world_path)
    model_name = name_of_tag("model", model_path)

    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{
            "robot_description": open(model_path, 'r', encoding="utf-8").read()
        }]
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", rviz_config_path]
    )

    left_front_wheel_rotating = Node(
        package="making_delivery",
        executable="left_front_wheel",
        output="screen"
    )

    left_wheels_node = Node(
        package="making_delivery",
        executable="left_wheels",
        output="screen"
    )

    lid_node = Node(
        package="making_delivery",
        executable="lid",
        output="screen"
    )

    gazebo_world_launcher = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory("ros_gz_sim"), "launch", "gz_sim.launch.py")
        ),
        launch_arguments={"gz_args": world_path}.items()
    )

    gazebo_spawn_node = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=["-world", world_name, "-file", model_path, model_name, "-x", "0", "-y", "-30", "-z", "0"]
    )

    gazebo_spawn_node1 = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=["-world", world_name, "-file", model_path1, model_name + "_1", "-x", "30", "-y", "10", "-z", "0"]
    )

    gazebo_spawn_node2 = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=["-world", world_name, "-file", model_path2, model_name + "_2", "-x", "-30", "-y", "15", "-z", "0"]
    )

    ros_to_gazebo_forces_bridge_node = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[f"/world/{world_name}/wrench@ros_gz_interfaces/msg/EntityWrench]gz.msgs.EntityWrench"]
    )

    gazebo_to_ros_coordinates_bridge_node = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[f"/world/{world_name}/pose/info@geometry_msgs/msg/PoseArray[gz.msgs.Pose_V"]
    )

    return LaunchDescription([
        SetEnvironmentVariable("GZ_SIM_RESOURCE_PATH", os.path.join(package_path, "textures")),
        # robot_state_publisher_node,
        # rviz_node,
        gazebo_world_launcher,
        gazebo_spawn_node,
        gazebo_spawn_node1,
        gazebo_spawn_node2,
        ros_to_gazebo_forces_bridge_node,
        gazebo_to_ros_coordinates_bridge_node,
        lid_node,
        #left_wheels_node
    ])
