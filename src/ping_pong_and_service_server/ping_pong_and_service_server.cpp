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
#include "std_srvs/srv/set_bool.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;


// Wait idly for a start command through the service
// Ping once every period if no pong was received
// Ping every time a pong is received, try to keep a configurable message rate, wait for a pong or timeout
// Stop after a number of messages defined in a parameter is received counting from the first pong


class PingPongNode : public rclcpp::Node
{
public:
  PingPongNode(const std::string &node_name, const std::string &pub_topic_name,
  const std::string &sub_topic_name, const std::string &service_name,
  std::chrono::milliseconds period)
  : Node(node_name), pub_count_(0), sub_count_(0), last_sub_count_(0),
  on_entry_(true), is_running_(false), period_(period)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>(pub_topic_name, 10);

    timer_ = this->create_wall_timer(
      10ms, std::bind(&PingPongNode::publisher_timer_callback, this));

    subscription_ = this->create_subscription<std_msgs::msg::String>(
      sub_topic_name, 10, std::bind(&PingPongNode::subscription_callback, this, _1));

    service_ = this->create_service<std_srvs::srv::SetBool>(
      service_name, std::bind(&PingPongNode::start, this, std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Node is ready and waiting");
  }

private:
  void publisher_timer_callback()
  {
    if(!is_running_)
    {
      on_entry_ = true;
      pub_count_ = 0;
      sub_count_ = 0;
      last_sub_count_ = 0;
      return;
    }

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
    if(is_running_)
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
  }

  void start(const std::shared_ptr<std_srvs::srv::SetBool::Request> request,
            std::shared_ptr<std_srvs::srv::SetBool::Response>     response)
  {
    is_running_ = request->data;
    response->success = true;
    response->message = "";
    if(request->data)
    {
      RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "A server request has arrived, starting execution");
      timer_->cancel();
      timer_ = this->create_wall_timer(
        10ms, std::bind(&PingPongNode::publisher_timer_callback, this));
    }else
    {
      RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "A server request has arrived, stoping execution");
      timer_->cancel();
      timer_ = this->create_wall_timer(
        period_, std::bind(&PingPongNode::publisher_timer_callback, this));
    }
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr service_;
  size_t pub_count_, sub_count_, last_sub_count_;
  bool on_entry_;
  bool is_running_;
  std::chrono::milliseconds period_;
};

int main(int argc, char * argv[])
{
  const std::string node_name = "ping_pong_and_server_node";
  const std::string pub_topic_name = "ping_topic";
  const std::string sub_topic_name = "pong_topic";
  const std::string service_name = "start";
  std::chrono::milliseconds period = 5000ms;

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PingPongNode>(node_name, pub_topic_name, sub_topic_name, service_name, period));
  rclcpp::shutdown();
  return 0;
}