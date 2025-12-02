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
#include <cstdint>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;


// Wait idly for a start true command through the service
// Ping once every period if no pong was received
// Ping every time a pong is received
// Consider a message is lost if x ms has passed
// Stop when a start false command is received through the service

// If already running, ignore new start commands

// Parameters
// Intended ping period
// Timeout value for delcaring a message as lost
// Number of messages exchanged



class PingPongNode : public rclcpp::Node
{
public:

  PingPongNode(const std::string &node_name, const std::string &pub_topic_name,
  const std::string &sub_topic_name, const std::string &service_name,
  std::chrono::milliseconds period)
  : Node(node_name), period_(period),
  pub_count_(0), sub_count_(0), last_sub_count_(0),
  on_entry_(true), is_running_(false), n_messages_(10), timeout_tick_limit_(1)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>(pub_topic_name, 1);

    subscription_ = this->create_subscription<std_msgs::msg::String>(
      sub_topic_name, 10, std::bind(&PingPongNode::subscription_callback, this, _1));

    service_ = this->create_service<std_srvs::srv::Trigger>(
      service_name, std::bind(&PingPongNode::start, this, std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Node is ready and waiting");

    // TODO: Add parameters to set n_messages_, timeout_tick_limit_ and period_
  }

private:
  rclcpp::TimerBase::SharedPtr timer_; // Timeout timer
  std::chrono::milliseconds period_;

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr service_;

  uint32_t pub_count_, sub_count_, last_sub_count_;
  bool on_entry_;
  bool is_running_;
  uint32_t n_messages_;
  uint8_t timeout_tick_limit_;


  // Timer callback used to publish messages
  void publisher_timer_callback()
  {
    if(!is_running_)
    {
      on_entry_ = true;
      pub_count_ = 0;
      sub_count_ = 0;
      last_sub_count_ = 0;
      timer_->cancel();
      return;
    }

    if(on_entry_)
    {
      timer_->cancel();
      timer_ = this->create_wall_timer(
        period_, std::bind(&PingPongNode::publisher_timer_callback, this));
      on_entry_ = false;
    }else
    {
      if(sub_count_ == last_sub_count_)
      {
        timeout_tick_limit_ --;
        if(timeout_tick_limit_ == 0)
        {
          last_sub_count_ = sub_count_;
          timeout_tick_limit_ = 1;
          RCLCPP_INFO(this->get_logger(), "Response for %d timed out", pub_count_ - 1);
        }
      }else
      {
        last_sub_count_ = sub_count_;
      }
    }

    // Timeout routine for detection of lost packets
    if(pub_count_ >= n_messages_)
    {
      // End of test
      timer_->cancel();
      RCLCPP_INFO(this->get_logger(), "End of test:");
      RCLCPP_INFO(this->get_logger(), "Sent %d messages", pub_count_);
      RCLCPP_INFO(this->get_logger(), "Received %d messages", sub_count_);
      pub_count_ = 0;
      sub_count_ = 0;
      last_sub_count_ = 0;
      on_entry_ = true;
      is_running_ = false;
      return;
    }

    auto message = std_msgs::msg::String();
    message.data = "Hello, world! " + std::to_string(pub_count_);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
    publisher_->publish(message);
    pub_count_++;
  }

  // Subscription callback to receive the pong messages
  void subscription_callback(const std_msgs::msg::String & msg)
  {
    if(is_running_)
    {
      sub_count_++;
      RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg.data.c_str());
    }
  }

  // Service callback to start one round of latency test
  void start(const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
            std::shared_ptr<std_srvs::srv::Trigger::Response>       response)
  {
    (void) request;
    if(!is_running_)
    {
      response->success = true;
      response->message = "A service request has arrived, starting execution";
      RCLCPP_INFO(this->get_logger(), "A service request has arrived, starting execution");
      timeout_tick_limit_ = 1;
      pub_count_ = 0;
      sub_count_ = 0;
      last_sub_count_ = -1;
      is_running_ = true;
      timer_ = this->create_wall_timer(
        1ms, std::bind(&PingPongNode::publisher_timer_callback, this));
    }else
    {
      response->success = false;
      response->message = "A service request has arrived, but operation is already running";
      RCLCPP_INFO(this->get_logger(), "A service request has arrived, but operation is already running");
    }
  }

};

int main(int argc, char * argv[])
{
  const std::string node_name = "latency_test_ping_pong_node";
  const std::string pub_topic_name = "ping_topic";
  const std::string sub_topic_name = "pong_topic";
  const std::string service_name = "start";
  std::chrono::milliseconds period = 1000ms;

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PingPongNode>(node_name, pub_topic_name, sub_topic_name, service_name, period));
  rclcpp::shutdown();
  return 0;
}