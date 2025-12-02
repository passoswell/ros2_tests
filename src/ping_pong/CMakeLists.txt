find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(ping_pong ping_pong.cpp)
ament_target_dependencies(ping_pong rclcpp std_msgs)

install(TARGETS ping_pong DESTINATION lib/${PROJECT_NAME})