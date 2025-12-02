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

class PingPongNode : public rclcpp::Node
{
public:
  PingPongNode(const std::string &node_name, const std::string &pub_topic_name,
  const std::string &sub_topic_name, std::chrono::milliseconds period)
  : Node(node_name), pub_count_(0), sub_count_(0), last_sub_count_(0),
  on_entry_(true), period_(period)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>(pub_topic_name, 10);

    timer_ = this->create_wall_timer(
      10ms, std::bind(&PingPongNode::publisher_timer_callback, this));

    subscription_ = this->create_subscription<std_msgs::msg::String>(
      sub_topic_name, 10, std::bind(&PingPongNode::subscription_callback, this, _1));
  }

private:
  void publisher_timer_callback()
  {
    if(!on_entry_)
    {
      if(sub_count_ != last_sub_count_)
      {
        sub_count_ = last_sub_count_;
        return;
      }
    }else
    {
      timer_->cancel();
      timer_ = this->create_wall_timer(
        period_, std::bind(&PingPongNode::publisher_timer_callback, this));
      on_entry_ = false;
    }
    auto message = std_msgs::msg::String();
    message.data = "Hello, world! " + std::to_string(pub_count_);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
    publisher_->publish(message);
    pub_count_++;
  }

  void subscription_callback(const std_msgs::msg::String & msg)
  {
    sub_count_++;
    RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg.data.c_str());

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    auto message = std_msgs::msg::String();
    message.data = "Hello, world! " + std::to_string(pub_count_);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
    publisher_->publish(message);
    pub_count_++;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  size_t pub_count_, sub_count_, last_sub_count_;
  bool on_entry_;
  std::chrono::milliseconds period_;
};

int main(int argc, char * argv[])
{
  const std::string node_name = "ping_pong_node";
  const std::string pub_topic_name = "ping_topic";
  const std::string sub_topic_name = "pong_topic";
  std::chrono::milliseconds period = 5000ms;

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PingPongNode>(node_name, pub_topic_name, sub_topic_name, period));
  rclcpp::shutdown();
  return 0;
}