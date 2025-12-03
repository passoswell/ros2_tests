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
#include <climits>

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
  on_entry_(true), is_running_(false), n_messages_(10),
  timeout_tick_limit_(1), timeout_ticks(1), payload_size_(10)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>(pub_topic_name, 1);

    subscription_ = this->create_subscription<std_msgs::msg::String>(
      sub_topic_name, 10, std::bind(&PingPongNode::subscription_callback, this, _1));

    service_ = this->create_service<std_srvs::srv::Trigger>(
      service_name, std::bind(&PingPongNode::start, this, std::placeholders::_1, std::placeholders::_2));

    auto param_desc = rcl_interfaces::msg::ParameterDescriptor{};

    param_desc.description = "Intended ping period in ms";
    this->declare_parameter("ping_rate", 100l, param_desc);

    param_desc.description = "Timeout value for delcaring a message as lost in ms";
    this->declare_parameter("pong_timeout", 200l, param_desc);

    param_desc.description = "Number of messages exchanged";
    this->declare_parameter("n_messages", 10l, param_desc);

    param_desc.description = "Number of bytes in the message";
    this->declare_parameter("payload_size", 10l, param_desc);

    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Node is ready and waiting");
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
  uint8_t timeout_tick_limit_, timeout_ticks;
  uint32_t payload_size_;

  std_msgs::msg::String message_;


  // Timer callback used to publish messages
  void publisher_timer_callback()
  {
    if(!is_running_)
    {
      on_entry_ = true;
      pub_count_ = 0;
      sub_count_ = 0;
      last_sub_count_ = sub_count_;
      timer_->cancel();
      return;
    }

    if(on_entry_)
    {
      timeout_ticks = timeout_tick_limit_;
      timer_->cancel();
      timer_ = this->create_wall_timer(
        period_, std::bind(&PingPongNode::publisher_timer_callback, this));
      on_entry_ = false;
    }else
    {
      if(sub_count_ == last_sub_count_)
      {
        timeout_ticks --;
        if(timeout_ticks == 0)
        {
          last_sub_count_ = sub_count_;
          timeout_ticks = timeout_tick_limit_;
          RCLCPP_INFO(this->get_logger(), "Response for %d timed out", pub_count_ - 1);
        }else
        {
          return;
        }
      }else
      {
        last_sub_count_ = sub_count_;
        timeout_ticks = timeout_tick_limit_;
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
      last_sub_count_ = sub_count_;
      on_entry_ = true;
      is_running_ = false;
      return;
    }

    if(((pub_count_ % 100) == 0) && (pub_count_ != 0))
    {
      float percentage = static_cast<float>(pub_count_) / static_cast<float>(n_messages_);
      RCLCPP_INFO(this->get_logger(), "%.2f%% done", 100 * percentage);
    }

    publisher_->publish(message_);
    pub_count_++;
  }

  // Subscription callback to receive the pong messages
  void subscription_callback(const std_msgs::msg::String & msg)
  {
    (void) msg;
    if(is_running_)
    {
      sub_count_++;
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
      RCLCPP_INFO(this->get_logger(), " ");
      RCLCPP_INFO(this->get_logger(), "A service request has arrived, starting execution");
      int64_t period = this->get_parameter("ping_rate").as_int();
      int64_t timeout = this->get_parameter("pong_timeout").as_int();
      int64_t n_messages = this->get_parameter("n_messages").as_int();
      int64_t payload_size = this->get_parameter("payload_size").as_int();
      timeout_tick_limit_ = timeout/period;
      if(timeout_tick_limit_ <= 1)
      {
        timeout_tick_limit_ = 2;
      }
      if(n_messages > 0 && n_messages < INT_MAX)
      {
        n_messages_ = static_cast<uint32_t>(n_messages);
      }else
      {
        n_messages_ = 10;
      }
      if(period >= 0.0)
      {
        period_ = period * 1ms;
      }
      if(payload_size > 0)
      {
        payload_size_ = payload_size;
      }
      RCLCPP_INFO(this->get_logger(), "payload_size: %d bytes (not counting ros2 / transport overhead)", payload_size_);
      RCLCPP_INFO(this->get_logger(), "Transmission rate: %ld ms (not enforced)", period_.count());
      RCLCPP_INFO(this->get_logger(), "Timeout_value: %ld ms (not enforced)", (period_ * timeout_tick_limit_).count());
      pub_count_ = 0;
      sub_count_ = 0;
      last_sub_count_ = sub_count_;
      message_.data = std::string(payload_size, 'A');
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