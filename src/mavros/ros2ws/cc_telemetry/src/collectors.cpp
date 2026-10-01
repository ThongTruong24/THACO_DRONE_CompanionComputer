#include <unistd.h>
#include "cc_telemetry/collectors.hpp"
#include <cstdlib>
#include <fstream>
#include <map>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <limits>
namespace cc {
namespace {
template<typename T> T get(const YAML::Node & n,const char * key,T fallback) {
 return n && n[key] ? n[key].as<T>() : fallback;
}
std::string env(const char * name,const char * fallback) {const char * p=std::getenv(name);return p?p:fallback;}
std::string ip(const char * name) {
 ifaddrs * all=nullptr;if(getifaddrs(&all))return "";
 std::string result;
 for(auto p=all;p;p=p->ifa_next) if(p->ifa_addr && p->ifa_addr->sa_family==AF_INET && std::string(p->ifa_name)==name) {
  char out[INET_ADDRSTRLEN]{};auto addr=reinterpret_cast<sockaddr_in *>(p->ifa_addr);
  if(inet_ntop(AF_INET,&addr->sin_addr,out,sizeof(out)))result=out;
 }
 freeifaddrs(all);return result;
}
}
Snapshot Collectors::collect(const std::string & group,bool connected) const {
 using J=nlohmann::json;
 if(group=="links") {
  std::string fc_port = env("DRONE_SERIAL_PORT","/dev/ttyAMA4");
  std::string siyi_port = env("SIYI_SERIAL_PORT","/dev/ttyAMA0");

  // 0 = DISCONNECTED, 1 = STANDBY, 2 = CONNECTED, 3 = ERROR
  int fc_status = 0;
  if (::access(fc_port.c_str(), F_OK) != 0) {
    fc_status = 0; // Port device does not exist
  } else if (::access(fc_port.c_str(), R_OK | W_OK) != 0) {
    fc_status = 3; // Permission denied or locked
  } else if (connected) {
    fc_status = 2; // Active MAVLink communication
  } else {
    fc_status = 1; // Port ready, waiting for FC signal
  }

  int siyi_status = 0;
  if (::access(siyi_port.c_str(), F_OK) != 0) {
    siyi_status = 0; // Port device does not exist
  } else if (::access(siyi_port.c_str(), R_OK | W_OK) != 0) {
    siyi_status = 3; // Permission denied or locked
  } else {
    siyi_status = connected ? 2 : 1;
  }

  J j = {
    {"fc_baudrate", std::stoul(env("DRONE_BAUD_RATE","921600"))},
    {"siyi_baudrate", std::stoul(env("SIYI_BAUD","115200"))},
    {"fc_bytes_rx", 0},
    {"fc_bytes_tx", 0},
    {"fc_bitrate_kbps", 0.0},
    {"fc_status", fc_status},
    {"siyi_status", siyi_status},
    {"link_status_flags", connected ? 1 : 0},
    {"fc_port", fc_port},
    {"siyi_port", siyi_port}
  };
  return {j, "FC status=" + std::to_string(fc_status) + ", SIYI status=" + std::to_string(siyi_status)};
 }
 if(group=="camera") {
  auto c=YAML::LoadFile(config_["camera_config"].as<std::string>());
  auto v=c["video"],e=c["encoder"],d=c["device_settings"]["realsense"];
  const auto profile=get<std::string>(d,"profile_mode","rgb_only");
  J j={{"video_width",get<int>(v,"width",0)},{"video_height",get<int>(v,"height",0)},
  {"rotation",get<int>(v,"rotation",0)},{"depth_width",get<int>(d,"depth_width",0)},
  {"depth_height",get<int>(d,"depth_height",0)},{"bitrate_kbps",get<int>(e,"bitrate_kbps",0)},
  {"bitrate_max_kbps",get<int>(e,"bitrate_max_kbps",0)},{"vbv_buffer_kb",get<int>(e,"vbv_buffer_kb",0)},
  {"video_fps",get<int>(v,"fps",0)},{"depth_fps",get<int>(d,"depth_fps",0)},
  {"profile_mode",profile=="rgb_only"?0:profile=="rgb_depth"?1:2},
  {"enable_emitter",get<bool>(d,"enable_emitter",false)?1:0},
  {"camera_type",get<std::string>(c,"camera_type","")},{"serial_number",get<std::string>(d,"serial_number","")},
  {"codec",get<std::string>(e,"codec","")},{"encoder_mode",get<std::string>(e,"mode","")},
  {"rtsp_url",config_["rtsp_url"].as<std::string>()}};
  return {j,"configured snapshot only; effective profile/FPS/auto serial unverified"};
 }
 if(group=="network") {
  std::ifstream f(config_["hostapd_config"].as<std::string>());if(!f)throw std::runtime_error("hostapd config unavailable");
  std::map<std::string,std::string> m;std::string line;
  while(std::getline(f,line)) {if(line.empty()||line[0]=='#')continue;auto n=line.find('=');if(n!=std::string::npos)m[line.substr(0,n)]=line.substr(n+1);}
  auto num=[&](const std::string & k){return m.count(k)?std::stoi(m[k]):0;};
  J j={{"ap_channel",num("channel")},{"ap_ieee80211n",num("ieee80211n")},{"ap_wmm_enabled",num("wmm_enabled")},
  {"ap_wpa",num("wpa")},{"ap_client_count",255},{"wlan0_dhcp",255},{"dnsmasq_status",255},
  {"eth0_ip",ip("eth0")},{"wlan0_ip",ip("wlan0")},{"ap_ip",ip("uap0")},
  {"ap_ssid",m["ssid"]},{"ap_wpa_passphrase",""},{"ap_key_mgmt",m["wpa_key_mgmt"]},{"ap_hw_mode",m["hw_mode"]}};
  return {j,"IPs measured; AP config configured; 255=unknown client count/DHCP/service; password redacted"};
 }
 if(group=="vision") {
  auto c=YAML::LoadFile(config_["vision_config"].as<std::string>());auto y=c["yolo"];
  auto model=get<std::string>(y,"model","");model=model.substr(model.find_last_of('/')+1);
  J j={{"confidence_thresh",get<double>(y,"confidence",0)},{"inference_fps",-1.0},
  {"input_width",get<int>(y,"width",0)},{"input_height",get<int>(y,"height",0)},
  {"video_fps",get<int>(y,"fps",0)},{"detections_count",255},{"status_flags",0},
  {"model_name",model},{"input_source",get<std::string>(y,"input_source","")}};
  return {j,"configured snapshot; inference_fps=-1/detections=255 unavailable; status not asserted"};
 }
 throw std::invalid_argument("Unknown collector");
}
}
