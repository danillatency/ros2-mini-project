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

add_executable(left_front_wheel src/left_front_wheel.cpp)
ament_target_dependencies(left_front_wheel rclcpp geometry_msgs tf2_ros tf2_geometry_msgs)

install(TARGETS
        left_front_wheel
        DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
        launch
        robots
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
