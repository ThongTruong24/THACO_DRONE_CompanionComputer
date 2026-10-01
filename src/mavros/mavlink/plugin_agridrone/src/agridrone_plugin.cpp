#include "plugin_agridrone/agridrone_plugin.hpp"
#include <mavros/mavros_plugin_register_macro.hpp>
#include <sstream>
#include "plugin_agridrone/telemetry_codec.hpp"
#include <dlfcn.h>

// Global dialect entry interceptor for MAVLink v2 CRC validation
namespace mavlink {
const mavlink_msg_entry_t *mavlink_get_msg_entry(uint32_t msgid)
{
  for (const auto & entry : mavlink::thaco_common::MESSAGE_ENTRIES) {
    if (entry.msgid == msgid) {
      return &entry;
    }
  }

  using Fn = const mavlink_msg_entry_t *(*)(uint32_t);
  static Fn orig = reinterpret_cast<Fn>(dlsym(RTLD_NEXT, "_ZN7mavlink21mavlink_get_msg_entryEj"));
  if (orig) {
    return orig(msgid);
  }
  return nullptr;
}
}  // namespace mavlink

namespace mavros {
namespace extra_plugins {

AgridronePlugin::AgridronePlugin(mavros::plugin::UASPtr uas_)
: Plugin(uas_, "agridrone")
{
  auto qos = rclcpp::SensorDataQoS();
  parameter_requests_ = node->create_publisher<cc_interfaces::msg::ParameterRequest>("cc/parameter_request",10);
  parameter_responses_ = node->create_subscription<cc_interfaces::msg::ParameterResponse>("cc/parameter_response",10,
    std::bind(&AgridronePlugin::parameter_response,this,std::placeholders::_1));
  cc_events_ = node->create_subscription<cc_interfaces::msg::Event>("cc/event",10,
    std::bind(&AgridronePlugin::cc_event,this,std::placeholders::_1));


  pub_xyz_trigger_ = node->create_publisher<std_msgs::msg::String>("thaco/xyz_trigger", qos);
  pub_links_ = node->create_publisher<std_msgs::msg::String>("cc_telemetry/links", qos);
  pub_camera_ = node->create_publisher<std_msgs::msg::String>("cc_telemetry/camera", qos);
  pub_network_ = node->create_publisher<std_msgs::msg::String>("cc_telemetry/network", qos);
  pub_vision_ = node->create_publisher<std_msgs::msg::String>("cc_telemetry/vision", qos);
  pub_command_ = node->create_publisher<std_msgs::msg::String>("cc_telemetry/command", qos);

  sub_send_xyz_trigger_ = node->create_subscription<std_msgs::msg::UInt32>(
    "thaco/send_xyz_trigger", 10,
    std::bind(&AgridronePlugin::send_xyz_trigger_cb, this, std::placeholders::_1));

  sub_send_links_ = node->create_subscription<std_msgs::msg::String>(
    "cc_telemetry/send_links", 10,
    std::bind(&AgridronePlugin::send_links_cb, this, std::placeholders::_1));

  sub_send_camera_ = node->create_subscription<std_msgs::msg::String>(
    "cc_telemetry/send_camera", 10,
    std::bind(&AgridronePlugin::send_camera_cb, this, std::placeholders::_1));

  sub_send_network_ = node->create_subscription<std_msgs::msg::String>(
    "cc_telemetry/send_network", 10,
    std::bind(&AgridronePlugin::send_network_cb, this, std::placeholders::_1));

  sub_send_vision_ = node->create_subscription<std_msgs::msg::String>(
    "cc_telemetry/send_vision", 10,
    std::bind(&AgridronePlugin::send_vision_cb, this, std::placeholders::_1));

  RCLCPP_INFO(get_logger(), "AgridronePlugin initialized successfully for THACO AgriDrone.");
}

mavros::plugin::Plugin::Subscriptions AgridronePlugin::get_subscriptions()
{
  return {
    make_handler(&AgridronePlugin::cc_param_request_list),
    make_handler(&AgridronePlugin::cc_param_request_read),
    make_handler(&AgridronePlugin::cc_param_set),
    make_handler(&AgridronePlugin::cc_param_ext_request_list),
    make_handler(&AgridronePlugin::cc_param_ext_request_read),
    make_handler(&AgridronePlugin::cc_param_ext_set),
    make_handler(&AgridronePlugin::handle_xyz_trigger),
    make_handler(&AgridronePlugin::handle_telemetry_links),
    make_handler(&AgridronePlugin::handle_telemetry_camera),
    make_handler(&AgridronePlugin::handle_telemetry_network),
    make_handler(&AgridronePlugin::handle_telemetry_vision),
    make_handler(&AgridronePlugin::handle_command_long),
  };
}

void AgridronePlugin::handle_xyz_trigger(
  const mavlink::mavlink_message_t * msg,
  mavlink::thaco_common::msg::THACO_EXTERNAL_XYZ_TRIGGER & trig,
  mavros::plugin::filter::AnyOk filter)
{
  (void)msg;
  (void)filter;
  std::ostringstream ss;
  ss << "{"
     << "\"trigger_id\":" << trig.trigger_id << ","
     << "\"time_boot_ms\":" << trig.time_boot_ms
     << "}";

  auto out = std_msgs::msg::String();
  out.data = ss.str();
  pub_xyz_trigger_->publish(out);

  RCLCPP_INFO_THROTTLE(
    get_logger(), *get_clock(), 2000,
    "Received THACO_EXTERNAL_XYZ_TRIGGER: trigger_id=%u, time_boot_ms=%u",
    trig.trigger_id, trig.time_boot_ms);
}

void AgridronePlugin::handle_telemetry_links(
  const mavlink::mavlink_message_t * msg,
  mavlink::thaco_common::msg::CC_TELEMETRY_LINKS & l,
  mavros::plugin::filter::AnyOk filter)
{
  (void)msg;
  (void)filter;
  std::string fc_port = mavlink::to_string(l.fc_port);
  std::string siyi_port = mavlink::to_string(l.siyi_port);

  std::ostringstream ss;
  ss << "{"
     << "\"fc_baudrate\":" << l.fc_baudrate << ","
     << "\"siyi_baudrate\":" << l.siyi_baudrate << ","
     << "\"fc_bytes_rx\":" << l.fc_bytes_rx << ","
     << "\"fc_bytes_tx\":" << l.fc_bytes_tx << ","
     << "\"fc_bitrate_kbps\":" << l.fc_bitrate_kbps << ","
     << "\"fc_status\":" << static_cast<int>(l.fc_status) << ","
     << "\"siyi_status\":" << static_cast<int>(l.siyi_status) << ","
     << "\"link_status_flags\":" << static_cast<int>(l.link_status_flags) << ","
     << "\"fc_port\":\"" << fc_port << "\","
     << "\"siyi_port\":\"" << siyi_port << "\""
     << "}";

  auto out = std_msgs::msg::String();
  out.data = ss.str();
  pub_links_->publish(out);
}

void AgridronePlugin::handle_telemetry_camera(
  const mavlink::mavlink_message_t * msg,
  mavlink::thaco_common::msg::CC_TELEMETRY_CAMERA & c,
  mavros::plugin::filter::AnyOk filter)
{
  (void)msg;
  (void)filter;
  std::string cam_type = mavlink::to_string(c.camera_type);
  std::string serial = mavlink::to_string(c.serial_number);
  std::string codec = mavlink::to_string(c.codec);
  std::string enc_mode = mavlink::to_string(c.encoder_mode);
  std::string rtsp_qgc = mavlink::to_string(c.rtsp_url_qgc);
  std::string rtsp_ctrl = mavlink::to_string(c.rtsp_url_controller);
  std::string rtsp_lap = mavlink::to_string(c.rtsp_url_laptop);

  std::ostringstream ss;
  ss << "{"
     << "\"video_width\":" << c.video_width << ","
     << "\"video_height\":" << c.video_height << ","
     << "\"rotation\":" << c.rotation << ","
     << "\"depth_width\":" << c.depth_width << ","
     << "\"depth_height\":" << c.depth_height << ","
     << "\"bitrate_kbps\":" << c.bitrate_kbps << ","
     << "\"bitrate_max_kbps\":" << c.bitrate_max_kbps << ","
     << "\"vbv_buffer_kb\":" << c.vbv_buffer_kb << ","
     << "\"video_fps\":" << static_cast<int>(c.video_fps) << ","
     << "\"depth_fps\":" << static_cast<int>(c.depth_fps) << ","
     << "\"profile_mode\":" << static_cast<int>(c.profile_mode) << ","
     << "\"enable_emitter\":" << static_cast<int>(c.enable_emitter) << ","
     << "\"camera_type\":\"" << cam_type << "\","
     << "\"serial_number\":\"" << serial << "\","
     << "\"codec\":\"" << codec << "\","
     << "\"encoder_mode\":\"" << enc_mode << "\","
     << "\"rtsp_url_qgc\":\"" << rtsp_qgc << "\","
     << "\"rtsp_url_controller\":\"" << rtsp_ctrl << "\","
     << "\"rtsp_url_laptop\":\"" << rtsp_lap << "\""
     << "}";

  auto out = std_msgs::msg::String();
  out.data = ss.str();
  pub_camera_->publish(out);
}

void AgridronePlugin::handle_telemetry_network(
  const mavlink::mavlink_message_t * msg,
  mavlink::thaco_common::msg::CC_TELEMETRY_NETWORK & n,
  mavros::plugin::filter::AnyOk filter)
{
  (void)msg;
  (void)filter;
  std::string eth0 = mavlink::to_string(n.eth0_ip);
  std::string wlan0 = mavlink::to_string(n.wlan0_ip);
  std::string ap = mavlink::to_string(n.ap_ip);
  std::string ssid = mavlink::to_string(n.ap_ssid);
  std::string psk = mavlink::to_string(n.ap_wpa_passphrase);
  std::string key_mgmt = mavlink::to_string(n.ap_key_mgmt);
  std::string hw_mode = mavlink::to_string(n.ap_hw_mode);

  std::ostringstream ss;
  ss << "{"
     << "\"ap_channel\":" << static_cast<int>(n.ap_channel) << ","
     << "\"ap_ieee80211n\":" << static_cast<int>(n.ap_ieee80211n) << ","
     << "\"ap_wmm_enabled\":" << static_cast<int>(n.ap_wmm_enabled) << ","
     << "\"ap_wpa\":" << static_cast<int>(n.ap_wpa) << ","
     << "\"ap_client_count\":" << static_cast<int>(n.ap_client_count) << ","
     << "\"wlan0_dhcp\":" << static_cast<int>(n.wlan0_dhcp) << ","
     << "\"dnsmasq_status\":" << static_cast<int>(n.dnsmasq_status) << ","
     << "\"eth0_ip\":\"" << eth0 << "\","
     << "\"wlan0_ip\":\"" << wlan0 << "\","
     << "\"ap_ip\":\"" << ap << "\","
     << "\"ap_ssid\":\"" << ssid << "\","
     << "\"ap_wpa_passphrase\":\"" << psk << "\","
     << "\"ap_key_mgmt\":\"" << key_mgmt << "\","
     << "\"ap_hw_mode\":\"" << hw_mode << "\""
     << "}";

  auto out = std_msgs::msg::String();
  out.data = ss.str();
  pub_network_->publish(out);
}

void AgridronePlugin::handle_telemetry_vision(
  const mavlink::mavlink_message_t * msg,
  mavlink::thaco_common::msg::CC_TELEMETRY_VISION & v,
  mavros::plugin::filter::AnyOk filter)
{
  (void)msg;
  (void)filter;
  std::string model = mavlink::to_string(v.model_name);
  std::string src = mavlink::to_string(v.input_source);

  std::ostringstream ss;
  ss << "{"
     << "\"confidence_thresh\":" << v.confidence_thresh << ","
     << "\"inference_fps\":" << v.inference_fps << ","
     << "\"input_width\":" << v.input_width << ","
     << "\"input_height\":" << v.input_height << ","
     << "\"video_fps\":" << static_cast<int>(v.video_fps) << ","
     << "\"detections_count\":" << static_cast<int>(v.detections_count) << ","
     << "\"status_flags\":" << static_cast<int>(v.status_flags) << ","
     << "\"model_name\":\"" << model << "\","
     << "\"input_source\":\"" << src << "\""
     << "}";

  auto out = std_msgs::msg::String();
  out.data = ss.str();
  pub_vision_->publish(out);
}

void AgridronePlugin::send_xyz_trigger_cb(const std_msgs::msg::UInt32::SharedPtr msg)
{
  mavlink::thaco_common::msg::THACO_EXTERNAL_XYZ_TRIGGER trig{};
  trig.trigger_id = msg->data;
  trig.time_boot_ms = get_time_boot_ms();

  uas->send_message(trig, 191);

  RCLCPP_INFO(
    get_logger(),
    "Sent MAVLink THACO_EXTERNAL_XYZ_TRIGGER: trigger_id=%u, time_boot_ms=%u",
    trig.trigger_id, trig.time_boot_ms);
}

void AgridronePlugin::send_links_cb(const std_msgs::msg::String::SharedPtr msg)
{
  try { auto packet=cc_wire::decode_links(msg->data); uas->send_message(packet,191); }
  catch(const std::exception &) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000, "CC links rejected: invalid payload");
  }
}

void AgridronePlugin::send_camera_cb(const std_msgs::msg::String::SharedPtr msg)
{
  try { auto packet=cc_wire::decode_camera(msg->data); uas->send_message(packet,191); }
  catch(const std::exception &) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000, "CC camera rejected: invalid payload");
  }
}

void AgridronePlugin::send_network_cb(const std_msgs::msg::String::SharedPtr msg)
{
  try { auto packet=cc_wire::decode_network(msg->data); uas->send_message(packet,191); }
  catch(const std::exception &) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000, "CC network rejected: invalid payload");
  }
}

void AgridronePlugin::send_vision_cb(const std_msgs::msg::String::SharedPtr msg)
{
  try { auto packet=cc_wire::decode_vision(msg->data); uas->send_message(packet,191); }
  catch(const std::exception &) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000, "CC vision rejected: invalid payload");
  }
}

void AgridronePlugin::cc_param_request_list(
 const mavlink::mavlink_message_t * raw, mavlink::common::msg::PARAM_REQUEST_LIST & m,
 mavros::plugin::filter::AnyOk)
{
 if(m.target_system!=1 || (m.target_component!=191 && !(0!=2 && m.target_component==0)))return;
 cc_interfaces::msg::ParameterRequest r;
 r.request_id=++request_sequence_;r.source_system=raw->sysid;r.source_component=raw->compid;
 r.target_system=m.target_system;r.target_component=m.target_component;
 r.operation=0;r.extended=false;r.param_index=-1;
 parameter_requests_->publish(r);
}
void AgridronePlugin::cc_param_request_read(
 const mavlink::mavlink_message_t * raw, mavlink::common::msg::PARAM_REQUEST_READ & m,
 mavros::plugin::filter::AnyOk)
{
 if(m.target_system!=1 || (m.target_component!=191 && !(1!=2 && m.target_component==0)))return;
 cc_interfaces::msg::ParameterRequest r;
 r.request_id=++request_sequence_;r.source_system=raw->sysid;r.source_component=raw->compid;
 r.target_system=m.target_system;r.target_component=m.target_component;
 r.operation=1;r.extended=false;r.param_index=-1;
 r.param_id=mavlink::to_string(m.param_id);
 r.param_index=m.param_index;
 parameter_requests_->publish(r);
}
void AgridronePlugin::cc_param_set(
 const mavlink::mavlink_message_t * raw, mavlink::common::msg::PARAM_SET & m,
 mavros::plugin::filter::AnyOk)
{
 if(m.target_system!=1 || (m.target_component!=191 && !(2!=2 && m.target_component==0)))return;
 cc_interfaces::msg::ParameterRequest r;
 r.request_id=++request_sequence_;r.source_system=raw->sysid;r.source_component=raw->compid;
 r.target_system=m.target_system;r.target_component=m.target_component;
 r.operation=2;r.extended=false;r.param_index=-1;
 r.param_id=mavlink::to_string(m.param_id);
 r.param_type=m.param_type;
 std::ostringstream value;value.precision(9);value<<m.param_value;r.value=value.str();
 parameter_requests_->publish(r);
}
void AgridronePlugin::cc_param_ext_request_list(
 const mavlink::mavlink_message_t * raw, mavlink::common::msg::PARAM_EXT_REQUEST_LIST & m,
 mavros::plugin::filter::AnyOk)
{
 if(m.target_system!=1 || (m.target_component!=191 && !(0!=2 && m.target_component==0)))return;
 cc_interfaces::msg::ParameterRequest r;
 r.request_id=++request_sequence_;r.source_system=raw->sysid;r.source_component=raw->compid;
 r.target_system=m.target_system;r.target_component=m.target_component;
 r.operation=0;r.extended=true;r.param_index=-1;
 parameter_requests_->publish(r);
}
void AgridronePlugin::cc_param_ext_request_read(
 const mavlink::mavlink_message_t * raw, mavlink::common::msg::PARAM_EXT_REQUEST_READ & m,
 mavros::plugin::filter::AnyOk)
{
 if(m.target_system!=1 || (m.target_component!=191 && !(1!=2 && m.target_component==0)))return;
 cc_interfaces::msg::ParameterRequest r;
 r.request_id=++request_sequence_;r.source_system=raw->sysid;r.source_component=raw->compid;
 r.target_system=m.target_system;r.target_component=m.target_component;
 r.operation=1;r.extended=true;r.param_index=-1;
 r.param_id=mavlink::to_string(m.param_id);
 r.param_index=m.param_index;
 parameter_requests_->publish(r);
}
void AgridronePlugin::cc_param_ext_set(
 const mavlink::mavlink_message_t * raw, mavlink::common::msg::PARAM_EXT_SET & m,
 mavros::plugin::filter::AnyOk)
{
 if(m.target_system!=1 || (m.target_component!=191 && !(2!=2 && m.target_component==0)))return;
 cc_interfaces::msg::ParameterRequest r;
 r.request_id=++request_sequence_;r.source_system=raw->sysid;r.source_component=raw->compid;
 r.target_system=m.target_system;r.target_component=m.target_component;
 r.operation=2;r.extended=true;r.param_index=-1;
 r.param_id=mavlink::to_string(m.param_id);
 r.param_type=m.param_type;
 r.value=mavlink::to_string(m.param_value);
 parameter_requests_->publish(r);
}

void AgridronePlugin::parameter_response(const cc_interfaces::msg::ParameterResponse::SharedPtr r)
{
 try {
  if(r->param_id.size()>16 || r->value.size()>128 || r->param_type!=9)return;
  if(r->extended && r->acknowledgement) {
   mavlink::common::msg::PARAM_EXT_ACK m{};
   mavlink::set_string(m.param_id,r->param_id);mavlink::set_string(m.param_value,r->value);
   m.param_type=r->param_type;m.param_result=r->result;uas->send_message(m,191);
  } else if(r->extended) {
   mavlink::common::msg::PARAM_EXT_VALUE m{};
   mavlink::set_string(m.param_id,r->param_id);mavlink::set_string(m.param_value,r->value);
   m.param_type=r->param_type;m.param_count=r->param_count;m.param_index=r->param_index;
   uas->send_message(m,191);
  } else {
   mavlink::common::msg::PARAM_VALUE m{};
   mavlink::set_string(m.param_id,r->param_id);m.param_value=std::stof(r->value);
   if(!std::isfinite(m.param_value))return;
   m.param_type=r->param_type;m.param_count=r->param_count;m.param_index=r->param_index;
   uas->send_message(m,191);
  }
 }catch(const std::exception &){RCLCPP_WARN_THROTTLE(get_logger(),*get_clock(),5000,"Invalid CC parameter response");}
}
void AgridronePlugin::cc_event(const cc_interfaces::msg::Event::SharedPtr e)
{
 const auto now=std::chrono::steady_clock::now();
 if(now-last_cc_event_<std::chrono::seconds(1))return;
 if(e->severity>7 || e->text.size()>50 || e->text.rfind("CC ",0)!=0)return;
 last_cc_event_=now;
 mavlink::common::msg::STATUSTEXT m{};m.severity=e->severity;
 mavlink::set_string(m.text,e->text);uas->send_message(m,191);
}


void AgridronePlugin::handle_command_long(
  const mavlink::mavlink_message_t * msg,
  mavlink::common::msg::COMMAND_LONG & cmd,
  mavros::plugin::filter::AnyOk filter)
{
  (void)msg;
  (void)filter;

  if (cmd.target_component != 191 && cmd.target_component != 0) {
    return;
  }

  if (cmd.command >= 44010 && cmd.command <= 44012) {
    std::ostringstream ss;
    ss << "{"
       << "\"command\":" << cmd.command << ","
       << "\"param1\":" << cmd.param1 << ","
       << "\"param2\":" << cmd.param2 << ","
       << "\"param3\":" << cmd.param3 << ","
       << "\"param4\":" << cmd.param4 << ","
       << "\"param5\":" << cmd.param5 << ","
       << "\"param6\":" << cmd.param6 << ","
       << "\"param7\":" << cmd.param7
       << "}";

    auto out = std_msgs::msg::String();
    out.data = ss.str();
    pub_command_->publish(out);

    mavlink::common::msg::COMMAND_ACK ack{};
    ack.command = cmd.command;
    ack.result = static_cast<uint8_t>(mavlink::common::MAV_RESULT::ACCEPTED);
    ack.progress = 100;
    ack.result_param2 = 0;
    ack.target_system = msg->sysid;
    ack.target_component = msg->compid;

    uas->send_message(ack, 191);

    RCLCPP_INFO(
      get_logger(),
      "Handled THACO command %u (target_comp=%u, param1=%f, param2=%f). Sent COMMAND_ACK ACCEPTED.",
      cmd.command, cmd.target_component, cmd.param1, cmd.param2);
  }
}

}  // namespace extra_plugins
}  // namespace mavros

MAVROS_PLUGIN_REGISTER(mavros::extra_plugins::AgridronePlugin)
