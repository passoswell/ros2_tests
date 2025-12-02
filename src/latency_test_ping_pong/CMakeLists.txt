find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)

add_executable(latency_test_ping_pong latency_test_ping_pong.cpp)
ament_target_dependencies(latency_test_ping_pong rclcpp std_msgs std_srvs)

install(TARGETS latency_test_ping_pong DESTINATION lib/${PROJECT_NAME})