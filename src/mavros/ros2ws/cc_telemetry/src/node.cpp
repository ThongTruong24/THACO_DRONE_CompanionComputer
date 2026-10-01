#include "cc_telemetry/collectors.hpp"
#include "cc_telemetry/parameter_store.hpp"
#include "cc_telemetry/system_metrics.hpp"

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/msg/debug_value.hpp>
#include <mavros_msgs/msg/status_text.hpp>
#include <mavros_msgs/msg/mavlink.hpp>
#include <cc_interfaces/msg/parameter_request.hpp>
#include <cc_interfaces/msg/parameter_response.hpp>
#include <cc_interfaces/msg/event.hpp>

#include <chrono>
#include <deque>
#include <fstream>
#include <iomanip>
#include <sstream>
// UNIX socket log aggregation (POSIX, no extra deps)
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

namespace cc {

using Request = cc_interfaces::msg::ParameterRequest;
using Response = cc_interfaces::msg::ParameterResponse;
using Clock = std::chrono::steady_clock;

class TelemetryNode : public rclcpp::Node {
 public:
  TelemetryNode() : Node("cc_telemetry") {
    const auto path = declare_parameter<std::string>("config_file", "/etc/cc-telemetry/cc_telemetry.yaml");
    config_ = YAML::LoadFile(path);

    collectors_ = std::make_unique<Collectors>(config_);
    store_ = std::make_unique<ParameterStore>(config_["state_directory"].as<std::string>());
    system_metrics_ = std::make_unique<SystemMetrics>();

    allow_remote_writes_ = config_["allow_remote_writes"] ? config_["allow_remote_writes"].as<bool>() : true;

    for (const auto & group : groups_) {
      const std::string key = rate_names_.at(group);
      double default_rate = config_["rates"][group] ? config_["rates"][group].as<double>() : 1.0;
      store_->add({key, default_rate, 0.0, 10.0, false, true}); // 0.0 = OFF
      pubs_[group] = create_publisher<std_msgs::msg::String>("/drone_mavros/cc_telemetry/send_" + group, 10);
    }

    // Default camera & vision settings
    double cam_w = 1280.0, cam_h = 720.0, cam_fps = 30.0, cam_kbps = 4000.0;
    try {
      auto cam = collectors_->collect("camera", false).values;
      if (cam.contains("video_width")) cam_w = cam.at("video_width").get<double>();
      if (cam.contains("video_height")) cam_h = cam.at("video_height").get<double>();
      if (cam.contains("video_fps")) cam_fps = cam.at("video_fps").get<double>();
      if (cam.contains("bitrate_kbps")) cam_kbps = cam.at("bitrate_kbps").get<double>();
    } catch (const std::exception & e) {
      RCLCPP_WARN(get_logger(), "Camera config note: %s", e.what());
    }
    store_->add({"CC_CAM_W", cam_w, 1.0, 4096.0, true, false});
    store_->add({"CC_CAM_H", cam_h, 1.0, 4096.0, true, false});
    store_->add({"CC_CAM_FPS", cam_fps, 1.0, 120.0, true, false});
    store_->add({"CC_CAM_KBPS", cam_kbps, 1.0, 65535.0, true, false});
    store_->add({"CC_CAM_ROT", 180.0, 0.0, 270.0, true, false});
    store_->add({"CC_CAM_RESET", 0.0, 0.0, 1.0, false, false});

    double yolo_conf = 0.45;
    try {
      auto vis = collectors_->collect("vision", false).values;
      if (vis.contains("confidence_thresh")) yolo_conf = vis.at("confidence_thresh").get<double>();
    } catch (const std::exception & e) {
      RCLCPP_WARN(get_logger(), "Vision config note: %s", e.what());
    }
    store_->add({"CC_YOLO_CONF", yolo_conf, 0.0, 1.0, false, false});

    // Control parameters: broadcast toggles and log severity
    store_->add({"CC_QGC_TEL", 1.0, 0.0, 1.0, true, true});    // 1=ON, 0=OFF
    store_->add({"CC_QGC_BANNER", 1.0, 0.0, 1.0, true, true}); // 1=ON, 0=OFF
    store_->add({"CC_LOG_SEV", static_cast<double>(log_min_sev_), 2.0, 6.0, true, true}); // 2..6

    try {
      store_->load();
    } catch (const std::exception & e) {
      RCLCPP_ERROR(get_logger(), "State load: %s", e.what());
    }

    // ROS 2 Communication
    responses_ = create_publisher<Response>("/drone_mavros/cc/parameter_response", 10);
    events_ = create_publisher<cc_interfaces::msg::Event>("/drone_mavros/cc/event", 10);
    diagnostics_ = create_publisher<std_msgs::msg::String>("~/diagnostics", 10);

    // QGC Standard telemetry publishers
    debug_val_pub_ = create_publisher<mavros_msgs::msg::DebugValue>("/drone_mavros/debug_value/send", 20);
    statustext_pub_ = create_publisher<mavros_msgs::msg::StatusText>("/drone_mavros/statustext/send", 10);

    // Subscriptions
    state_ = create_subscription<mavros_msgs::msg::State>(
        "/drone_mavros/state", rclcpp::SensorDataQoS(),
        [this](mavros_msgs::msg::State::ConstSharedPtr m) {
          connected_ = m->connected;
          state_time_ = Clock::now();
        });

    requests_ = create_subscription<Request>(
        "/drone_mavros/cc/parameter_request", 10,
        [this](Request::ConstSharedPtr m) { handle(*m); });

    mavlink_raw_sub_ = create_subscription<mavros_msgs::msg::Mavlink>(
        "/drone_mavros/mavlink/from", 50,
        [this](mavros_msgs::msg::Mavlink::ConstSharedPtr m) {
          uint32_t b = m->payload64.size() * 8;
          mavlink_bytes_accum_ += b;
          total_fc_bytes_rx_ += b;
        });

    sub_draft_camera_ = create_subscription<std_msgs::msg::String>(
        "/drone_mavros/cc_telemetry/camera", 10,
        [this](std_msgs::msg::String::ConstSharedPtr m) { handle_draft_camera(m->data); });

    sub_draft_vision_ = create_subscription<std_msgs::msg::String>(
        "/drone_mavros/cc_telemetry/vision", 10,
        [this](std_msgs::msg::String::ConstSharedPtr m) { handle_draft_vision(m->data); });

    sub_draft_links_ = create_subscription<std_msgs::msg::String>(
        "/drone_mavros/cc_telemetry/links", 10,
        [this](std_msgs::msg::String::ConstSharedPtr m) { handle_draft_links(m->data); });

    sub_draft_network_ = create_subscription<std_msgs::msg::String>(
        "/drone_mavros/cc_telemetry/network", 10,
        [this](std_msgs::msg::String::ConstSharedPtr m) { handle_draft_network(m->data); });

    sub_command_ = create_subscription<std_msgs::msg::String>(
        "/drone_mavros/cc_telemetry/command", 10,
        [this](std_msgs::msg::String::ConstSharedPtr m) { handle_command(m->data); });

    tick_ = create_wall_timer(std::chrono::milliseconds(100), [this] { tick(); });

    // ── QGC Log Aggregation Socket ────────────────────────────────────────
    // Cau hinh tu qgc_log section trong cc_telemetry.yaml
    if (config_["qgc_log"]) {
      const auto & lg = config_["qgc_log"];
      if (lg["socket_path"]) log_sock_path_ = lg["socket_path"].as<std::string>();
      if (lg["max_msg_per_sec"]) log_max_per_sec_ = lg["max_msg_per_sec"].as<int>();
      if (lg["max_msg_len"])    log_max_len_     = lg["max_msg_len"].as<int>();
      if (lg["min_severity"])   log_min_sev_     = lg["min_severity"].as<int>();
    }
    init_log_socket();

    RCLCPP_INFO(get_logger(), "Unified CC & QGC Telemetry Node started (bidirectional Ras <-> GCS enabled, writes=%s)",
                allow_remote_writes_ ? "true" : "false");
  }

  ~TelemetryNode() override {
    if (log_sock_fd_ >= 0) {
      ::close(log_sock_fd_);
      ::unlink(log_sock_path_.c_str());
    }
  }

 private:
  static std::string number(double v) {
    std::ostringstream s;
    s << std::setprecision(9) << v;
    return s.str();
  }

  // ── QGC Log Aggregation: khoi tao UNIX datagram socket ──────────────────
  void init_log_socket() {
    // Xoa socket cu neu ton tai
    ::unlink(log_sock_path_.c_str());

    log_sock_fd_ = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (log_sock_fd_ < 0) {
      RCLCPP_WARN(get_logger(), "[log_sock] socket() failed: %s", strerror(errno));
      return;
    }

    // Dat non-blocking de khong block tick()
    int flags = ::fcntl(log_sock_fd_, F_GETFL, 0);
    if (flags >= 0) ::fcntl(log_sock_fd_, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, log_sock_path_.c_str(), sizeof(addr.sun_path) - 1);

    if (::bind(log_sock_fd_, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr)) < 0) {
      RCLCPP_WARN(get_logger(), "[log_sock] bind(%s) failed: %s",
                  log_sock_path_.c_str(), strerror(errno));
      ::close(log_sock_fd_);
      log_sock_fd_ = -1;
      return;
    }

    // Cho phep tat ca containers ghi vao socket
    ::chmod(log_sock_path_.c_str(), 0777);
    RCLCPP_INFO(get_logger(), "[log_sock] Listening on %s (min_sev=%d, max=%d/s)",
                log_sock_path_.c_str(), log_min_sev_, log_max_per_sec_);
  }

  // ── Doc va forward log datagram len QGC ─────────────────────────────────
  // Goi moi tick (100ms). Rate-limit: toi da log_max_per_sec_ msg/s.
  void drain_log_socket() {
    if (log_sock_fd_ < 0) return;

    const auto now = Clock::now();

    // Reset token bucket moi giay
    if (std::chrono::duration<double>(now - log_rate_window_).count() >= 1.0) {
      log_rate_window_ = now;
      log_tokens_used_ = 0;
    }

    // Doc toi da (log_max_per_sec_ - da dung) messages trong mot tick
    static constexpr size_t kBufSize = 1024;
    char buf[kBufSize];

    for (int i = 0; i < log_max_per_sec_ * 2; ++i) {
      if (log_tokens_used_ >= log_max_per_sec_) break;

      struct sockaddr_un client_addr{};
      socklen_t client_len = sizeof(client_addr);
      ssize_t n = ::recvfrom(log_sock_fd_, buf, kBufSize - 1, MSG_DONTWAIT,
                             reinterpret_cast<struct sockaddr *>(&client_addr), &client_len);
      if (n <= 0) break;  // EAGAIN/EWOULDBLOCK: het datagram

      buf[n] = '\0';
      try {
        auto j = nlohmann::json::parse(buf, buf + n);

        // Control command support over socket
        if (j.contains("cmd")) {
          const std::string cmd = j.value("cmd", "");
          if (cmd == "set_param") {
            const std::string pname = j.value("name", "");
            double pval = j.value("val", 0.0);
            auto p = store_->find(pname);
            nlohmann::json reply;
            if (p) {
              auto res = store_->set(pname, pval, allow_remote_writes_);
              if (pname == "CC_LOG_SEV" && res.accepted) {
                log_min_sev_ = static_cast<int>(pval);
              }
              reply = {{"ok", res.accepted}, {"reason", res.reason}, {"name", pname}, {"val", pval}};
              event("CC " + pname + " " + res.reason, res.accepted ? 6 : 4);
              Request fake_req{};
              fake_req.request_id = ++req_seq_;
              fake_req.target_system = 1;
              fake_req.target_component = 191;
              fake_req.operation = 1;
              enqueue(fake_req, *store_->find(pname));
            } else {
              reply = {{"ok", false}, {"reason", "unknown parameter"}};
            }
            if (client_len > 0 && client_addr.sun_path[0] != '\0') {
              std::string rep_str = reply.dump();
              ::sendto(log_sock_fd_, rep_str.data(), rep_str.size(), 0,
                       reinterpret_cast<const struct sockaddr *>(&client_addr), client_len);
            }
            continue;
          } else if (cmd == "get_params") {
            nlohmann::json reply = nlohmann::json::object();
            for (const auto & p : store_->all()) {
              reply[p.name] = {
                  {"val", p.value},
                  {"active", store_->active(p.name)},
                  {"min", p.minimum},
                  {"max", p.maximum},
                  {"live", p.live}
              };
            }
            if (client_len > 0 && client_addr.sun_path[0] != '\0') {
              std::string rep_str = reply.dump();
              ::sendto(log_sock_fd_, rep_str.data(), rep_str.size(), 0,
                       reinterpret_cast<const struct sockaddr *>(&client_addr), client_len);
            }
            continue;
          }
        }

        const int sev = j.value("sev", 6);
        if (sev > log_min_sev_) continue;  // loc severity (nho hon = nghiem trong hon)

        const std::string src = j.value("src", "?");
        const std::string msg = j.value("msg", "");
        if (msg.empty()) continue;

        // Format: "[src] msg", cat ngon neu vuot log_max_len_
        std::string text = "[" + src + "] " + msg;
        if (static_cast<int>(text.size()) > log_max_len_) {
          text = text.substr(0, static_cast<size_t>(log_max_len_));
        }

        RCLCPP_INFO(get_logger(), "[QGC-log] sev=%d %s", sev, text.c_str());
        send_statustext(text, static_cast<uint8_t>(sev));
        ++log_tokens_used_;

      } catch (const std::exception &) {
        // Datagram khong hop le -- bo qua
      }
    }
  }

  void send_named_value(const std::string & name, float value) {
    mavros_msgs::msg::DebugValue msg;
    msg.header.stamp = now();
    msg.type = mavros_msgs::msg::DebugValue::TYPE_NAMED_VALUE_FLOAT;
    msg.name = name.substr(0, 10);
    msg.value_float = value;
    debug_val_pub_->publish(msg);
  }

  void send_statustext(const std::string & text, uint8_t severity = 6) {
    mavros_msgs::msg::StatusText msg;
    msg.header.stamp = now();
    msg.severity = severity;
    msg.text = text.substr(0, 50);
    statustext_pub_->publish(msg);
  }

  void event(const std::string & text, uint8_t severity = 6) {
    RCLCPP_INFO(get_logger(), "%s", text.c_str());
    if (Clock::now() - last_event_ < std::chrono::seconds(1)) return;
    last_event_ = Clock::now();

    cc_interfaces::msg::Event e;
    e.severity = severity;
    e.text = text.substr(0, 50);
    events_->publish(e);

    // Also notify QGC
    send_statustext(text, severity);
  }

  void enqueue(const Request & req, const Parameter & p, bool ack = false, uint8_t result = 0) {
    if (queue_.size() >= 64) return;
    Response r;
    r.request_id = req.request_id;
    r.extended = req.extended;
    r.acknowledgement = ack;
    r.result = result;
    r.param_id = p.name;
    r.param_type = 9;
    r.value = number(p.value);
    r.param_count = store_->all().size();
    for (size_t i = 0; i < store_->all().size(); ++i) {
      if (store_->all()[i].name == p.name) r.param_index = i;
    }
    queue_.push_back(r);
  }

  void handle(const Request & req) {
    if (req.target_system != 1 || (req.target_component != 191 && !(req.operation != 2 && req.target_component == 0))) return;
    if (Clock::now() - last_request_ < std::chrono::milliseconds(50)) return;
    last_request_ = Clock::now();

    if (req.operation == 0) {
      if (!queue_.empty()) return;
      for (const auto & p : store_->all()) enqueue(req, p);
      return;
    }

    const Parameter * p = nullptr;
    if (req.operation == 1 && req.param_index >= 0) {
      if (static_cast<size_t>(req.param_index) < store_->all().size()) {
        p = &store_->all()[req.param_index];
      }
    } else {
      p = store_->find(req.param_id);
    }

    if (!p) {
      event("CC unknown/read-only parameter", 4);
      return;
    }

    if (req.operation == 1) {
      enqueue(req, *p);
      return;
    }

    if (req.operation != 2) return;

    const std::string name = p->name;
    SetResult result{false, false, "invalid value"};
    try {
      size_t used = 0;
      double v = std::stod(req.value, &used);
      if (used == req.value.size() || used > 0) {
        result = store_->set(name, v, allow_remote_writes_);
        if (name == "CC_LOG_SEV" && result.accepted) {
          log_min_sev_ = static_cast<int>(v);
        }
      }
    } catch (const std::exception &) {
      result = {false, false, "parse error"};
    }

    enqueue(req, *store_->find(name), req.extended, result.accepted ? 0 : 2);
    event("CC " + name + " " + result.reason, result.accepted ? 6 : 4);
    RCLCPP_INFO(get_logger(), "request=%lu source=%u.%u parameter=%s accepted=%d pending=%d",
                static_cast<unsigned long>(req.request_id), req.source_system, req.source_component,
                name.c_str(), result.accepted, result.pending);
  }

  void write_camera_command() {
    try {
      nlohmann::json cmd;
      cmd["bitrate_kbps"] = static_cast<int>(store_->active("CC_CAM_KBPS"));
      cmd["rotation"] = static_cast<int>(store_->active("CC_CAM_ROT"));
      if (store_->active("CC_CAM_RESET") > 0.5) {
        cmd["reset"] = true;
        store_->set("CC_CAM_RESET", 0.0, true);
      } else {
        cmd["reset"] = false;
      }
      cmd["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();

      std::ofstream f("/run/drone/camera_command.json");
      if (f.is_open()) {
        f << cmd.dump(2);
      }
    } catch (const std::exception & e) {
      RCLCPP_WARN(get_logger(), "Failed to write camera_command: %s", e.what());
    }
  }

  void read_camera_stats(float & kbps, float & fps, float & drop, bool & is_standby) {
    kbps = -1.0f;
    fps = -1.0f;
    drop = -1.0f;
    is_standby = false;
    std::ifstream f("/run/drone/camera_stats.json");
    if (!f.is_open()) return;
    try {
      nlohmann::json j;
      f >> j;
      if (j.contains("bitrate_kbps")) kbps = j["bitrate_kbps"].get<float>();
      if (j.contains("measured_fps")) fps = j["measured_fps"].get<float>();
      if (j.contains("dropped_frames")) drop = j["dropped_frames"].get<float>();
      if (j.contains("is_standby")) is_standby = j["is_standby"].get<bool>();
    } catch (...) {}
  }

  void read_boot_status(float & stage, float & ctn_ok, float & ctn_fail) {
    stage = -1.0f;
    ctn_ok = -1.0f;
    ctn_fail = -1.0f;
    std::ifstream f("/run/drone/boot_status.json");
    if (!f.is_open()) return;
    try {
      nlohmann::json j;
      f >> j;
      if (j.contains("stage")) stage = j["stage"].get<float>();
      if (j.contains("containers_ready")) ctn_ok = j["containers_ready"].get<float>();
      if (j.contains("containers_failed")) ctn_fail = j["containers_failed"].get<float>();
    } catch (...) {}
  }

  void broadcast_qgc_telemetry() {
    system_metrics_->update();

    // 1. System Metrics (Total CPU, Cores, RAM, Temp)
    send_named_value("PI_CPU", system_metrics_->total_cpu());
    send_named_value("PI_RAM", system_metrics_->ram_percentage());
    send_named_value("PI_TEMP", system_metrics_->temperature_c());

    const auto & cores = system_metrics_->per_core_cpu();
    for (size_t i = 0; i < cores.size() && i < 4; ++i) {
      send_named_value("PI_C" + std::to_string(i), cores[i]);
    }

    // 2. MAVLink Bitrate
    send_named_value("ML_KBPS", mavlink_kbps_);

    // 3. Camera & Vision Metrics (Live for QGC Inspector)
    float cam_kbps, cam_fps, cam_drop;
    bool is_standby = false;
    read_camera_stats(cam_kbps, cam_fps, cam_drop, is_standby);

    if (cam_fps >= 0.0f) {
      send_named_value("CAM_FPS", cam_fps);
    } else {
      send_named_value("CAM_FPS", static_cast<float>(store_->active("CC_CAM_FPS")));
    }

    if (cam_kbps >= 0.0f) {
      send_named_value("CAM_KBPS", cam_kbps);
    } else {
      send_named_value("CAM_KBPS", static_cast<float>(store_->active("CC_CAM_KBPS")));
    }

    if (cam_drop >= 0.0f) {
      send_named_value("CAM_DROP", cam_drop);
    }

    send_named_value("VIS_CONF", static_cast<float>(store_->active("CC_YOLO_CONF")));

    if (last_vision_json_.contains("inference_fps") && last_vision_json_["inference_fps"].get<double>() >= 0) {
      send_named_value("VIS_FPS", static_cast<float>(last_vision_json_["inference_fps"].get<double>()));
    }
    if (last_vision_json_.contains("detections_count") && last_vision_json_["detections_count"].get<int>() != 255) {
      send_named_value("VIS_DET", static_cast<float>(last_vision_json_["detections_count"].get<int>()));
    }

    // 4. Boot Stage
    float b_stg, b_ok, b_fail;
    read_boot_status(b_stg, b_ok, b_fail);
    if (b_stg >= 0.0f) send_named_value("BOOT_STG", b_stg);
    if (b_ok >= 0.0f) send_named_value("CTN_OK", b_ok);
    if (b_fail >= 0.0f) send_named_value("CTN_FAIL", b_fail);
  }

  void broadcast_qgc_banner() {
    static int banner_mode = 0;
    if (banner_mode == 0) {
      char buf[64];
      std::snprintf(buf, sizeof(buf), "PI: CPU %.0f%% | T:%.0fC | RAM:%lu/%luMB",
                    system_metrics_->total_cpu(),
                    system_metrics_->temperature_c(),
                    static_cast<unsigned long>(system_metrics_->ram_used_mb()),
                    static_cast<unsigned long>(system_metrics_->ram_total_mb()));
      send_statustext(buf, 6);
      banner_mode = 1;
    } else {
      char buf[64];
      float cam_kbps, cam_fps, cam_drop;
      bool is_standby = false;
    read_camera_stats(cam_kbps, cam_fps, cam_drop, is_standby);
      if (cam_kbps >= 0.0f) {
        std::snprintf(buf, sizeof(buf), "CAM: %.0fkbps@%.0ffps | Drop:%lu | YOLO:%.2f",
                      cam_kbps, cam_fps >= 0 ? cam_fps : store_->active("CC_CAM_FPS"),
                      static_cast<unsigned long>(cam_drop >= 0 ? cam_drop : 0),
                      store_->active("CC_YOLO_CONF"));
      } else {
        std::snprintf(buf, sizeof(buf), "CAM: %.0fx%.0f@%.0ffps | YOLO conf:%.2f",
                      store_->active("CC_CAM_W"),
                      store_->active("CC_CAM_H"),
                      store_->active("CC_CAM_FPS"),
                      store_->active("CC_YOLO_CONF"));
      }
      send_statustext(buf, 6);
      banner_mode = 0;
    }
  }

  void tick() {
    if (!queue_.empty()) {
      responses_->publish(queue_.front());
      queue_.pop_front();
    }

    // Drain log socket moi tick (100ms), rate-limited
    drain_log_socket();

    const auto now = Clock::now();

    // Calculate MAVLink bitrate every 1.0s
    if (std::chrono::duration<double>(now - last_bitrate_calc_).count() >= 1.0) {
      double dt = std::chrono::duration<double>(now - last_bitrate_calc_).count();
      mavlink_kbps_ = static_cast<float>((mavlink_bytes_accum_ * 8.0) / (dt * 1000.0));
      mavlink_bytes_accum_ = 0;
      last_bitrate_calc_ = now;
    }

    // Publish QGC Named Values (1.0 Hz) - check CC_QGC_TEL
    if (store_->active("CC_QGC_TEL") >= 1.0 &&
        std::chrono::duration<double>(now - last_qgc_named_values_).count() >= 1.0) {
      last_qgc_named_values_ = now;
      broadcast_qgc_telemetry();
    }

    // Publish QGC StatusText banner (every 10s) - check CC_QGC_BANNER
    if (store_->active("CC_QGC_BANNER") >= 1.0 &&
        std::chrono::duration<double>(now - last_qgc_banner_).count() >= 10.0) {
      last_qgc_banner_ = now;
      broadcast_qgc_banner();
    }

    // Publish THACO Custom MAVLink groups (0.0 = OFF)
    for (const auto & group : groups_) {
      double rate = store_->active(rate_names_.at(group));
      if (rate <= 0.0) {
        continue; // Group is disabled
      }
      if (std::chrono::duration<double>(now - last_[group]).count() < 1.0 / rate) {
        continue;
      }
      last_[group] = now;
      try {
        bool fresh = connected_ && (now - state_time_ < std::chrono::seconds(5));
        auto s = collectors_->collect(group, fresh);
        if (group == "vision") last_vision_json_ = s.values;
        if (group == "camera") last_camera_json_ = s.values;
      if (group == "links") {
          s.values["fc_bitrate_kbps"] = mavlink_kbps_;
          s.values["fc_bytes_rx"] = total_fc_bytes_rx_;

          int fc_st = s.values.value("fc_status", 0);
          int siyi_st = s.values.value("siyi_status", 0);

          if (fc_st != last_fc_status_) {
            last_fc_status_ = fc_st;
            std::string fc_port = s.values.value("fc_port", "/dev/ttyAMA4");
            if (fc_st == 2) {
              send_statustext("FC Link: ONLINE (" + fc_port + ")", 6);
            } else if (fc_st == 1) {
              send_statustext("FC Link: STANDBY (" + fc_port + ")", 5);
            } else if (fc_st == 0) {
              send_statustext("FC Link: DISCONNECTED (" + fc_port + ")", 4);
            } else {
              send_statustext("FC Link: ERROR (" + fc_port + ")", 3);
            }
          }

          if (siyi_st != last_siyi_status_) {
            last_siyi_status_ = siyi_st;
            std::string siyi_port = s.values.value("siyi_port", "/dev/ttyAMA0");
            if (siyi_st == 2) {
              send_statustext("SIYI Link: ONLINE (" + siyi_port + ")", 6);
            } else if (siyi_st == 1) {
              send_statustext("SIYI Link: STANDBY (" + siyi_port + ")", 6);
            } else if (siyi_st == 0) {
              send_statustext("SIYI Link: DISCONNECTED (" + siyi_port + ")", 4);
            }
          }
        }

        std_msgs::msg::String out;
        out.data = s.values.dump();
        pubs_.at(group)->publish(out);
        diagnostic_state_[group] = {{"source_note", s.diagnostic}, {"available", true}};
      } catch (const std::exception & e) {
        diagnostic_state_[group] = {{"available", false}, {"error", e.what()}};
      }
    }

    // Diagnostics every 5s
    if (now - last_diagnostic_ > std::chrono::seconds(5)) {
      last_diagnostic_ = now;
      nlohmann::json state = diagnostic_state_;
      for (const auto & p : store_->all()) {
        state["parameters"][p.name] = {
            {"desired", p.value},
            {"active_baseline", store_->active(p.name)},
            {"live", p.live}};
      }
      std_msgs::msg::String d;
      d.data = state.dump();
      diagnostics_->publish(d);
    }
  }


  void send_group_telemetry(const std::string & group) {
    try {
      auto now = Clock::now();
      bool fresh = connected_ && (now - state_time_ < std::chrono::seconds(5));
      auto s = collectors_->collect(group, fresh);
      if (group == "vision") last_vision_json_ = s.values;
      if (group == "camera") last_camera_json_ = s.values;
      if (group == "links") {
          s.values["fc_bitrate_kbps"] = mavlink_kbps_;
          s.values["fc_bytes_rx"] = total_fc_bytes_rx_;

          int fc_st = s.values.value("fc_status", 0);
          int siyi_st = s.values.value("siyi_status", 0);

          if (fc_st != last_fc_status_) {
            last_fc_status_ = fc_st;
            std::string fc_port = s.values.value("fc_port", "/dev/ttyAMA4");
            if (fc_st == 2) {
              send_statustext("FC Link: ONLINE (" + fc_port + ")", 6);
            } else if (fc_st == 1) {
              send_statustext("FC Link: STANDBY (" + fc_port + ")", 5);
            } else if (fc_st == 0) {
              send_statustext("FC Link: DISCONNECTED (" + fc_port + ")", 4);
            } else {
              send_statustext("FC Link: ERROR (" + fc_port + ")", 3);
            }
          }

          if (siyi_st != last_siyi_status_) {
            last_siyi_status_ = siyi_st;
            std::string siyi_port = s.values.value("siyi_port", "/dev/ttyAMA0");
            if (siyi_st == 2) {
              send_statustext("SIYI Link: ONLINE (" + siyi_port + ")", 6);
            } else if (siyi_st == 1) {
              send_statustext("SIYI Link: STANDBY (" + siyi_port + ")", 6);
            } else if (siyi_st == 0) {
              send_statustext("SIYI Link: DISCONNECTED (" + siyi_port + ")", 4);
            }
          }
        }

      std_msgs::msg::String out;
      out.data = s.values.dump();
      if (pubs_.count(group)) {
        pubs_.at(group)->publish(out);
      }
    } catch (const std::exception & e) {
      RCLCPP_WARN(get_logger(), "Failed to send telemetry for %s: %s", group.c_str(), e.what());
    }
  }

  void handle_draft_camera(const std::string & json_str) {
    try {
      auto j = nlohmann::json::parse(json_str);
      if (j.contains("bitrate_kbps")) staged_cam_["bitrate_kbps"] = j["bitrate_kbps"].get<int>();
      if (j.contains("rotation")) staged_cam_["rotation"] = j["rotation"].get<int>();
      if (j.contains("video_width")) staged_cam_["video_width"] = j["video_width"].get<int>();
      if (j.contains("video_height")) staged_cam_["video_height"] = j["video_height"].get<int>();
      if (j.contains("video_fps")) staged_cam_["video_fps"] = j["video_fps"].get<int>();
      if (j.contains("codec")) staged_cam_["codec"] = j["codec"].get<std::string>();
      RCLCPP_INFO(get_logger(), "Stored staged Camera config: %s", j.dump().c_str());
    } catch (const std::exception & e) {
      RCLCPP_WARN(get_logger(), "Error parsing draft camera: %s", e.what());
    }
  }

  void handle_draft_vision(const std::string & json_str) {
    try {
      auto j = nlohmann::json::parse(json_str);
      if (j.contains("confidence_thresh")) staged_vis_["confidence_thresh"] = j["confidence_thresh"].get<float>();
      if (j.contains("model_name")) staged_vis_["model_name"] = j["model_name"].get<std::string>();
      if (j.contains("input_source")) staged_vis_["input_source"] = j["input_source"].get<std::string>();
      RCLCPP_INFO(get_logger(), "Stored staged Vision config: %s", j.dump().c_str());
    } catch (const std::exception & e) {
      RCLCPP_WARN(get_logger(), "Error parsing draft vision: %s", e.what());
    }
  }

  void handle_draft_links(const std::string & json_str) {
    try {
      auto j = nlohmann::json::parse(json_str);
      if (j.contains("fc_baudrate")) staged_links_["fc_baudrate"] = j["fc_baudrate"].get<int>();
      if (j.contains("siyi_baudrate")) staged_links_["siyi_baudrate"] = j["siyi_baudrate"].get<int>();
      if (j.contains("fc_port")) staged_links_["fc_port"] = j["fc_port"].get<std::string>();
      if (j.contains("siyi_port")) staged_links_["siyi_port"] = j["siyi_port"].get<std::string>();
      RCLCPP_INFO(get_logger(), "Stored staged Links config: %s", j.dump().c_str());
    } catch (const std::exception & e) {
      RCLCPP_WARN(get_logger(), "Error parsing draft links: %s", e.what());
    }
  }

  void handle_draft_network(const std::string & json_str) {
    try {
      auto j = nlohmann::json::parse(json_str);
      if (j.contains("ap_channel")) staged_net_["ap_channel"] = j["ap_channel"].get<int>();
      if (j.contains("ap_ssid")) staged_net_["ap_ssid"] = j["ap_ssid"].get<std::string>();
      if (j.contains("ap_wpa_passphrase")) staged_net_["ap_wpa_passphrase"] = j["ap_wpa_passphrase"].get<std::string>();
      if (j.contains("eth0_ip")) staged_net_["eth0_ip"] = j["eth0_ip"].get<std::string>();
      RCLCPP_INFO(get_logger(), "Stored staged Network config: %s", j.dump().c_str());
    } catch (const std::exception & e) {
      RCLCPP_WARN(get_logger(), "Error parsing draft network: %s", e.what());
    }
  }

  void handle_command(const std::string & json_str) {
    try {
      auto j = nlohmann::json::parse(json_str);
      uint32_t cmd = j.value("command", 0);
      int subsystem = static_cast<int>(j.value("param1", 0.0f));
      bool immediate = (j.value("param2", 0.0f) > 0.5f);

      RCLCPP_INFO(get_logger(), "Executing THACO command %u for subsystem %d (immediate=%d)", cmd, subsystem, immediate);

      if (cmd == 44011) { // MAV_CMD_THACO_APPLY_CONFIG
        if (subsystem == 2 || subsystem == 0) { // Camera
          if (!staged_cam_.empty()) {
            nlohmann::json cam_cmd;
            if (staged_cam_.contains("bitrate_kbps")) {
              cam_cmd["bitrate_kbps"] = staged_cam_["bitrate_kbps"];
              store_->set("CC_CAM_KBPS", static_cast<double>(staged_cam_["bitrate_kbps"].get<int>()), true);
            }
            if (staged_cam_.contains("rotation")) {
              cam_cmd["rotation"] = staged_cam_["rotation"];
              store_->set("CC_CAM_ROT", static_cast<double>(staged_cam_["rotation"].get<int>()), true);
            }
            cam_cmd["reset"] = false;
            cam_cmd["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

            std::ofstream f("/run/drone/camera_command.json");
            if (f.is_open()) {
              f << cam_cmd.dump(2);
              RCLCPP_INFO(get_logger(), "Hot reloaded camera via /run/drone/camera_command.json");
            }
          }
          send_group_telemetry("camera");
        }

        if (subsystem == 4 || subsystem == 0) { // Vision
          if (!staged_vis_.empty()) {
            if (staged_vis_.contains("confidence_thresh")) {
              store_->set("CC_YOLO_CONF", staged_vis_["confidence_thresh"].get<float>(), true);
            }
          }
          send_group_telemetry("vision");
        }

        if (subsystem == 1 || subsystem == 0) { // Links / Telemetry
          send_group_telemetry("links");
        }

        if (subsystem == 3 || subsystem == 0) { // Network
          send_group_telemetry("network");
        }
      } else if (cmd == 44010) { // MAV_CMD_THACO_SAVE_DEFAULT_CONFIG
        RCLCPP_INFO(get_logger(), "Saved default config for subsystem %d", subsystem);
      } else if (cmd == 44012) { // MAV_CMD_THACO_RESTORE_DEFAULT_CONFIG
        RCLCPP_INFO(get_logger(), "Restored default config for subsystem %d", subsystem);
      }
    } catch (const std::exception & e) {
      RCLCPP_ERROR(get_logger(), "Exception handling THACO command: %s", e.what());
    }
  }


  nlohmann::json staged_cam_;
  nlohmann::json staged_vis_;
  nlohmann::json staged_net_;
  nlohmann::json staged_links_;

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_draft_camera_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_draft_vision_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_draft_links_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_draft_network_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_command_;

  YAML::Node config_;
  std::unique_ptr<Collectors> collectors_;
  std::unique_ptr<ParameterStore> store_;
  std::unique_ptr<SystemMetrics> system_metrics_;
  bool allow_remote_writes_{true};

  std::vector<std::string> groups_{"links", "camera", "network", "vision"};
  std::map<std::string, std::string> rate_names_{
      {"links", "CC_LINK_HZ"}, {"camera", "CC_CAM_HZ"}, {"network", "CC_NET_HZ"}, {"vision", "CC_VIS_HZ"}};
  std::map<std::string, rclcpp::Publisher<std_msgs::msg::String>::SharedPtr> pubs_;
  std::map<std::string, Clock::time_point> last_;

  Clock::time_point state_time_{}, last_event_{}, last_request_{}, last_diagnostic_{};
  Clock::time_point last_qgc_named_values_{}, last_qgc_banner_{}, last_bitrate_calc_{};

  bool connected_{false};
  nlohmann::json diagnostic_state_;
  nlohmann::json last_camera_json_;
  nlohmann::json last_vision_json_;

  uint64_t req_seq_{0};
  uint64_t mavlink_bytes_accum_{0};
  float mavlink_kbps_{0.0f};
  uint32_t total_fc_bytes_rx_{0};
  int last_fc_status_{-1};
  int last_siyi_status_{-1};

  // ── QGC Log Socket members ────────────────────────────────────────────────
  std::string log_sock_path_{"/run/drone/log.sock"};
  int         log_sock_fd_{-1};
  int         log_max_per_sec_{5};
  int         log_max_len_{50};
  int         log_min_sev_{4};  // WARNING tro len (4=WARNING, 3=ERROR, 2=CRITICAL)
  int         log_tokens_used_{0};
  Clock::time_point log_rate_window_{};

  std::deque<Response> queue_;
  rclcpp::Publisher<Response>::SharedPtr responses_;
  rclcpp::Publisher<cc_interfaces::msg::Event>::SharedPtr events_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr diagnostics_;

  rclcpp::Publisher<mavros_msgs::msg::DebugValue>::SharedPtr debug_val_pub_;
  rclcpp::Publisher<mavros_msgs::msg::StatusText>::SharedPtr statustext_pub_;

  rclcpp::Subscription<Request>::SharedPtr requests_;
  rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr state_;
  rclcpp::Subscription<mavros_msgs::msg::Mavlink>::SharedPtr mavlink_raw_sub_;

  rclcpp::TimerBase::SharedPtr tick_;
};

}  // namespace cc

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  try {
    rclcpp::spin(std::make_shared<cc::TelemetryNode>());
  } catch (const std::exception & e) {
    fprintf(stderr, "CC startup failed: %s\n", e.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
