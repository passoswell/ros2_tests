find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(minimal_publisher minimal_publisher.cpp)
ament_target_dependencies(minimal_publisher rclcpp std_msgs)

install(TARGETS minimal_publisher DESTINATION lib/${PROJECT_NAME})