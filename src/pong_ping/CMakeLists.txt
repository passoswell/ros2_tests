find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(pong_ping pong_ping.cpp)
ament_target_dependencies(pong_ping rclcpp std_msgs)

install(TARGETS pong_ping DESTINATION lib/${PROJECT_NAME})