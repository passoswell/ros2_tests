find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)

add_executable(ping_pong_and_service_server ping_pong_and_service_server.cpp)
ament_target_dependencies(ping_pong_and_service_server rclcpp std_msgs std_srvs)

install(TARGETS ping_pong_and_service_server DESTINATION lib/${PROJECT_NAME})