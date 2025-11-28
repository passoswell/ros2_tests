find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(minimal_subscriber minimal_subscriber.cpp)
ament_target_dependencies(minimal_subscriber rclcpp std_msgs)

install(TARGETS minimal_subscriber DESTINATION lib/${PROJECT_NAME})