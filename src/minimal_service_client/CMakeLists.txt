find_package(rclcpp REQUIRED)
find_package(example_interfaces REQUIRED)

add_executable(minimal_service_client minimal_service_client.cpp)
ament_target_dependencies(minimal_service_client rclcpp example_interfaces)

install(TARGETS minimal_service_client DESTINATION lib/${PROJECT_NAME})