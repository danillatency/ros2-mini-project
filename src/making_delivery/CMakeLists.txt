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

rosidl_generate_interfaces(${PROJECT_NAME}
        srv/SummonDeliverer.srv
        DEPENDENCIES geometry_msgs
)


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

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
add_executable(summon_deliverer src/summon_deliverer_server.cpp)
target_link_libraries(summon_deliverer PUBLIC
        ${geometry_msgs_TARGETS}
        rclcpp::rclcpp
        "${cpp_typesupport_target}"
)


install(TARGETS
        left_wheels
        lid
        summon_deliverer
        DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
        launch
        robots
        models
        textures
        worlds
        rviz
        DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
