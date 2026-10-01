#pragma once
#include <nlohmann/json.hpp>
#include <cmath>
#include <limits>
#include <algorithm>
#include <stdexcept>
namespace cc_wire {
template<class T> void field(const nlohmann::json & j,const char * key,T & out) {
 const auto & v=j.at(key);
 if(!v.is_number())throw std::invalid_argument("Numeric field required");
 const double n=v.get<double>();
 if(!std::isfinite(n) || n<static_cast<double>(std::numeric_limits<T>::lowest()) ||
 n>static_cast<double>(std::numeric_limits<T>::max()) ||
 (std::numeric_limits<T>::is_integer && std::trunc(n)!=n))throw std::out_of_range("Field range");
 out=static_cast<T>(n);
}
template<size_t N> void field(const nlohmann::json & j,const char * key,std::array<char,N> & out) {
 const auto s=j.at(key).get<std::string>();
 if(s.size()>N || s.find('\0')!=std::string::npos)throw std::out_of_range("String field length");
 out.fill(0);std::copy(s.begin(),s.end(),out.begin());
}
inline mavlink::thaco_common::msg::CC_TELEMETRY_LINKS decode_links(const std::string & text) {
 if(text.size()>4096)throw std::invalid_argument("Payload too large");
 const auto j=nlohmann::json::parse(text);
 mavlink::thaco_common::msg::CC_TELEMETRY_LINKS out{};
 field(j,"fc_baudrate",out.fc_baudrate);
 field(j,"siyi_baudrate",out.siyi_baudrate);
 field(j,"fc_bytes_rx",out.fc_bytes_rx);
 field(j,"fc_bytes_tx",out.fc_bytes_tx);
 field(j,"fc_bitrate_kbps",out.fc_bitrate_kbps);
 field(j,"fc_status",out.fc_status);
 field(j,"siyi_status",out.siyi_status);
 field(j,"link_status_flags",out.link_status_flags);
 field(j,"fc_port",out.fc_port);
 field(j,"siyi_port",out.siyi_port);
 return out;
}
inline mavlink::thaco_common::msg::CC_TELEMETRY_CAMERA decode_camera(const std::string & text) {
 if(text.size()>4096)throw std::invalid_argument("Payload too large");
 const auto j=nlohmann::json::parse(text);
 mavlink::thaco_common::msg::CC_TELEMETRY_CAMERA out{};
 field(j,"video_width",out.video_width);
 field(j,"video_height",out.video_height);
 field(j,"rotation",out.rotation);
 field(j,"depth_width",out.depth_width);
 field(j,"depth_height",out.depth_height);
 field(j,"bitrate_kbps",out.bitrate_kbps);
 field(j,"bitrate_max_kbps",out.bitrate_max_kbps);
 field(j,"vbv_buffer_kb",out.vbv_buffer_kb);
 field(j,"video_fps",out.video_fps);
 field(j,"depth_fps",out.depth_fps);
 field(j,"profile_mode",out.profile_mode);
 field(j,"enable_emitter",out.enable_emitter);
 field(j,"camera_type",out.camera_type);
 field(j,"serial_number",out.serial_number);
 field(j,"codec",out.codec);
 field(j,"encoder_mode",out.encoder_mode);
 field(j,"rtsp_url_qgc",out.rtsp_url_qgc);
 field(j,"rtsp_url_controller",out.rtsp_url_controller);
 field(j,"rtsp_url_laptop",out.rtsp_url_laptop);
 return out;
}
inline mavlink::thaco_common::msg::CC_TELEMETRY_NETWORK decode_network(const std::string & text) {
 if(text.size()>4096)throw std::invalid_argument("Payload too large");
 const auto j=nlohmann::json::parse(text);
 mavlink::thaco_common::msg::CC_TELEMETRY_NETWORK out{};
 field(j,"ap_channel",out.ap_channel);
 field(j,"ap_ieee80211n",out.ap_ieee80211n);
 field(j,"ap_wmm_enabled",out.ap_wmm_enabled);
 field(j,"ap_wpa",out.ap_wpa);
 field(j,"ap_client_count",out.ap_client_count);
 field(j,"wlan0_dhcp",out.wlan0_dhcp);
 field(j,"dnsmasq_status",out.dnsmasq_status);
 field(j,"eth0_ip",out.eth0_ip);
 field(j,"wlan0_ip",out.wlan0_ip);
 field(j,"ap_ip",out.ap_ip);
 field(j,"ap_ssid",out.ap_ssid);
 out.ap_wpa_passphrase.fill(0); // Never transmit network secrets.
 field(j,"ap_key_mgmt",out.ap_key_mgmt);
 field(j,"ap_hw_mode",out.ap_hw_mode);
 return out;
}
inline mavlink::thaco_common::msg::CC_TELEMETRY_VISION decode_vision(const std::string & text) {
 if(text.size()>4096)throw std::invalid_argument("Payload too large");
 const auto j=nlohmann::json::parse(text);
 mavlink::thaco_common::msg::CC_TELEMETRY_VISION out{};
 field(j,"confidence_thresh",out.confidence_thresh);
 field(j,"inference_fps",out.inference_fps);
 field(j,"input_width",out.input_width);
 field(j,"input_height",out.input_height);
 field(j,"video_fps",out.video_fps);
 field(j,"detections_count",out.detections_count);
 field(j,"status_flags",out.status_flags);
 field(j,"model_name",out.model_name);
 field(j,"input_source",out.input_source);
 return out;
}
}
