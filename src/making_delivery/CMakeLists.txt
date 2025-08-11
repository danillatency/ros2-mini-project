cmake_minimum_required(VERSION 3.8)
project(making_delivery)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(ros_gz_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(ros_gz_bridge REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
        srv/SummonDeliverer.srv
        srv/SubscribeTransformStreamer.srv
        DEPENDENCIES geometry_msgs
)
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")

add_executable(left_wheels src/left_wheels.cpp)
target_link_libraries(left_wheels PUBLIC
        ${geometry_msgs_TARGETS}
        rclcpp::rclcpp
        tf2_ros::static_transform_broadcaster_node
        tf2_ros::tf2_ros
)

add_executable(lid src/lid.cpp)
target_link_libraries(lid PUBLIC
        ${geometry_msgs_TARGETS}
        ${ros_gz_interfaces_TARGETS}
        rclcpp::rclcpp
        tf2_geometry_msgs::tf2_geometry_msgs
        tf2_ros::static_transform_broadcaster_node
        tf2_ros::tf2_ros
)

add_executable(package_executor src/package_executor.cpp)
target_include_directories(package_executor PUBLIC include)
target_link_libraries(package_executor PUBLIC
        ${geometry_msgs_TARGETS}
        rclcpp::rclcpp
        ros_gz_bridge::ros_gz_bridge
        "${cpp_typesupport_target}"
)

#add_executable(streamer src/transform_streamer.cpp)
#target_link_libraries(streamer PUBLIC
#        ${geometry_msgs_TARGETS}
#        rclcpp::rclcpp
#        "${cpp_typesupport_target}"
#)

install(TARGETS
        left_wheels
        lid
        package_executor
        DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
        launch
        robots
        models
        textures
        worlds
        rviz
        bridge_templates
        DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
