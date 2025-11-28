#include <chrono>
#include <functional>
#include <string>

#include <rclcpp/rclcpp.hpp>

using namespace std::chrono_literals;

class MinimalParam : public rclcpp::Node
{
public:
  MinimalParam(const std::string &node_name, const std::string &parameter_name,
  const std::string &parameter_description, std::chrono::milliseconds period)
  : Node(node_name), parameter_name_(parameter_name)
  {
    auto param_desc = rcl_interfaces::msg::ParameterDescriptor{};
    param_desc.description = parameter_description;

    this->declare_parameter(parameter_name, "world", param_desc);

    timer_ = this->create_wall_timer(
      period, std::bind(&MinimalParam::timer_callback, this));
  }

  void timer_callback()
  {
    std::string my_param = this->get_parameter(parameter_name_).as_string();

    RCLCPP_INFO(this->get_logger(), "Hello %s!", my_param.c_str());

    std::vector<rclcpp::Parameter> all_new_parameters{rclcpp::Parameter(parameter_name_, my_param)};
    this->set_parameters(all_new_parameters);
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  const std::string &parameter_name_;
};

int main(int argc, char ** argv)
{
  const std::string node_name = "minimal_parameter";
  const std::string parameter_name = "my_parameter";
  const std::string parameter_description = "This parameter is mine!";
  std::chrono::milliseconds period = 1000ms;

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalParam>(node_name, parameter_name, parameter_description, period));
  rclcpp::shutdown();
  return 0;
}