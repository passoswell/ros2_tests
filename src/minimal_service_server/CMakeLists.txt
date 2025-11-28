find_package(rclcpp REQUIRED)
find_package(example_interfaces REQUIRED)

add_executable(minimal_service_server minimal_service_server.cpp)
ament_target_dependencies(minimal_service_server rclcpp example_interfaces)

install(TARGETS minimal_service_server DESTINATION lib/${PROJECT_NAME})