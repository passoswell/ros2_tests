// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

using namespace std::chrono_literals;



/* This example creates a subclass of Node and uses std::bind() to register a
 * member function as a callback from the timer. */

class MinimalPublisher : public rclcpp::Node
{
public:
  MinimalPublisher(const std::string &node_name, const std::string &topic_name,
  const std::string &service_name, std::chrono::milliseconds period)
  : Node(node_name), count_(0)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>(topic_name, 10);

    timer_ = this->create_wall_timer(
      period, std::bind(&MinimalPublisher::publisher_timer_callback, this));

    service_ = this->create_service<example_interfaces::srv::AddTwoInts>(
      service_name, std::bind(&MinimalPublisher::add, this, std::placeholders::_1,
        std::placeholders::_2));


    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Ready to add two ints.");
  }

private:
  void publisher_timer_callback()
  {
    auto message = std_msgs::msg::String();
    message.data = "Hello, world! " + std::to_string(count_);
    count_++;
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
    publisher_->publish(message);
  }

  void add(const std::shared_ptr<example_interfaces::srv::AddTwoInts::Request> request,
            std::shared_ptr<example_interfaces::srv::AddTwoInts::Response>     response)
  {
    response->sum = request->a + request->b;
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Incoming request\na: %ld" " b: %ld",
                  request->a, request->b);
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "sending back response: [%ld]", (long int)response->sum);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Service<example_interfaces::srv::AddTwoInts>::SharedPtr service_;
  size_t count_;
};

int main(int argc, char * argv[])
{
  const std::string node_name = "minimal_publisher_and_service_server";
  const std::string topic_name = "pubsub_topic";
  const std::string service_name = "add_two_ints";
  std::chrono::milliseconds period = 1000ms;

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalPublisher>(node_name, topic_name, service_name, period));
  rclcpp::shutdown();
  return 0;
}
