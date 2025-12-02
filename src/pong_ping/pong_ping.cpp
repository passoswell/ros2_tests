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



// Ping once every period if no pong was received
// Ping every time a pong is received

class PongPingNode : public rclcpp::Node
{
public:
  PongPingNode(const std::string &node_name, const std::string &pub_topic_name,
  const std::string &sub_topic_name)
  : Node(node_name), pub_count_(0), sub_count_(0)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>(pub_topic_name, 10);

    subscription_ = this->create_subscription<std_msgs::msg::String>(
      sub_topic_name, 10, std::bind(&PongPingNode::subscription_callback, this, _1));
  }

private:

  void subscription_callback(const std_msgs::msg::String & msg)
  {
    sub_count_++;
    RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg.data.c_str());
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    auto message = std_msgs::msg::String();
    message.data = "Hello, world! " + std::to_string(pub_count_);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
    publisher_->publish(message);
    pub_count_++;
  }

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  size_t pub_count_, sub_count_;
};

int main(int argc, char * argv[])
{
  const std::string node_name = "ping_pong_node";
  const std::string pub_topic_name = "pong_topic";
  const std::string sub_topic_name = "ping_topic";

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PongPingNode>(node_name, pub_topic_name, sub_topic_name));
  rclcpp::shutdown();
  return 0;
}