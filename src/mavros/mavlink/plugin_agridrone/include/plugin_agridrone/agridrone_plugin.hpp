#pragma once

#include <memory>
#include <chrono>
#include <cc_interfaces/msg/parameter_request.hpp>
#include <cc_interfaces/msg/parameter_response.hpp>
#include <cc_interfaces/msg/event.hpp>
#include <string>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/u_int32.hpp>

// Forward declaration to break circular dependency in mavros headers
namespace mavros {
namespace plugin {
class Plugin;
class PluginFactory;
}  // namespace plugin
}  // namespace mavros

#include <mavros/mavros_uas.hpp>
#include <mavros/plugin.hpp>
#include <mavros/plugin_filter.hpp>
#include <mavconn/interface.hpp>

#include <mavlink/v2.0/thaco_common/thaco_common.hpp>

namespace mavros {
namespace extra_plugins {

class AgridronePlugin : public mavros::plugin::Plugin {
public:
  RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(AgridronePlugin)

  explicit AgridronePlugin(mavros::plugin::UASPtr uas_);
  ~AgridronePlugin() override = default;

  Subscriptions get_subscriptions() override;

private:
  // MAVLink -> ROS 2 message handlers with auto-decoding
  void handle_xyz_trigger(
    const mavlink::mavlink_message_t * msg,
    mavlink::thaco_common::msg::THACO_EXTERNAL_XYZ_TRIGGER & trig,
    mavros::plugin::filter::AnyOk filter);

  void handle_telemetry_links(
    const mavlink::mavlink_message_t * msg,
    mavlink::thaco_common::msg::CC_TELEMETRY_LINKS & links,
    mavros::plugin::filter::AnyOk filter);

  void handle_telemetry_camera(
    const mavlink::mavlink_message_t * msg,
    mavlink::thaco_common::msg::CC_TELEMETRY_CAMERA & cam,
    mavros::plugin::filter::AnyOk filter);

  void handle_telemetry_network(
    const mavlink::mavlink_message_t * msg,
    mavlink::thaco_common::msg::CC_TELEMETRY_NETWORK & net,
    mavros::plugin::filter::AnyOk filter);

  void handle_telemetry_vision(
    const mavlink::mavlink_message_t * msg,
    mavlink::thaco_common::msg::CC_TELEMETRY_VISION & vis,
    mavros::plugin::filter::AnyOk filter);

  void handle_command_long(
    const mavlink::mavlink_message_t * msg,
    mavlink::common::msg::COMMAND_LONG & cmd,
    mavros::plugin::filter::AnyOk filter);

  // ROS 2 -> MAVLink callbacks
  void send_xyz_trigger_cb(const std_msgs::msg::UInt32::SharedPtr msg);
  void send_links_cb(const std_msgs::msg::String::SharedPtr msg);
  void send_camera_cb(const std_msgs::msg::String::SharedPtr msg);
  void send_network_cb(const std_msgs::msg::String::SharedPtr msg);
  void send_vision_cb(const std_msgs::msg::String::SharedPtr msg);


  rclcpp::Publisher<cc_interfaces::msg::ParameterRequest>::SharedPtr parameter_requests_;
  rclcpp::Subscription<cc_interfaces::msg::ParameterResponse>::SharedPtr parameter_responses_;
  rclcpp::Subscription<cc_interfaces::msg::Event>::SharedPtr cc_events_;
  uint64_t request_sequence_ = 0;
  std::chrono::steady_clock::time_point last_cc_event_{};
  void parameter_response(const cc_interfaces::msg::ParameterResponse::SharedPtr response);
  void cc_event(const cc_interfaces::msg::Event::SharedPtr event);
  void cc_param_request_list(const mavlink::mavlink_message_t *, mavlink::common::msg::PARAM_REQUEST_LIST &, mavros::plugin::filter::AnyOk);
  void cc_param_request_read(const mavlink::mavlink_message_t *, mavlink::common::msg::PARAM_REQUEST_READ &, mavros::plugin::filter::AnyOk);
  void cc_param_set(const mavlink::mavlink_message_t *, mavlink::common::msg::PARAM_SET &, mavros::plugin::filter::AnyOk);
  void cc_param_ext_request_list(const mavlink::mavlink_message_t *, mavlink::common::msg::PARAM_EXT_REQUEST_LIST &, mavros::plugin::filter::AnyOk);
  void cc_param_ext_request_read(const mavlink::mavlink_message_t *, mavlink::common::msg::PARAM_EXT_REQUEST_READ &, mavros::plugin::filter::AnyOk);
  void cc_param_ext_set(const mavlink::mavlink_message_t *, mavlink::common::msg::PARAM_EXT_SET &, mavros::plugin::filter::AnyOk);

  // ROS 2 publishers
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_xyz_trigger_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_links_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_camera_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_network_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_vision_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_command_;

  // ROS 2 subscribers
  rclcpp::Subscription<std_msgs::msg::UInt32>::SharedPtr sub_send_xyz_trigger_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_send_links_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_send_camera_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_send_network_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_send_vision_;
};

}  // namespace extra_plugins
}  // namespace mavros
