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

using namespace std::chrono_literals;
using std::placeholders::_1;



/* This example creates a subclass of Node and uses std::bind() to register a
 * member function as a callback from the timer. */

class MinimalNode : public rclcpp::Node
{
public:
  MinimalNode(const std::string &node_name, const std::string &pub_topic_name,
  const std::string &sub_topic_name, std::chrono::milliseconds period)
  : Node(node_name), count_(0)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>(pub_topic_name, 10);

    timer_ = this->create_wall_timer(
      period, std::bind(&MinimalNode::publisher_timer_callback, this));

    subscription_ = this->create_subscription<std_msgs::msg::String>(
      sub_topic_name, 10, std::bind(&MinimalNode::subscription_callback, this, _1));
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

  void subscription_callback(const std_msgs::msg::String & msg) const
  {
    RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg.data.c_str());
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  size_t count_;
};

int main(int argc, char * argv[])
{
  const std::string node_name = "minimal_publisher_and_subscriber";
  const std::string pub_topic_name = "pub_topic";
  const std::string sub_topic_name = "pubsub_topic";
  std::chrono::milliseconds period = 1000ms;

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalNode>(node_name, pub_topic_name, sub_topic_name, period));
  rclcpp::shutdown();
  return 0;
}