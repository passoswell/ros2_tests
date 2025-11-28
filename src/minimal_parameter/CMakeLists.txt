find_package(rclcpp REQUIRED)

add_executable(minimal_parameter minimal_parameter.cpp)
ament_target_dependencies(minimal_parameter rclcpp std_msgs)

install(TARGETS minimal_parameter DESTINATION lib/${PROJECT_NAME})