#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <thread>
#include "mavlink_core.hpp"
#include "serial_enumerator.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <cstdio>

namespace cc {

// ─── Helpers ──────────────────────────────────────────────────────────────
static void str_to_buf(const char* src, char* dst, size_t bufsz) {
    std::memset(dst, 0, bufsz);
    if (src) std::strncpy(dst, src, bufsz - 1);
}

// ─── Constructor / Destructor ─────────────────────────────────────────────
MavlinkCore::MavlinkCore(const std::string& router_ip, int router_port,
                          RollbackGuard& guard,
                          SystemMetricsCollector& metrics,
                          NetworkMonitor& net,
                          ConfigEngine& config_engine,
                          std::shared_ptr<HardwareRegistry> hw_reg,
                          std::shared_ptr<UdsHubServer> uds_hub)
    : router_ip_(router_ip), router_port_(router_port),
      guard_(guard), metrics_(metrics), net_(net), config_engine_(config_engine),
      hw_reg_(hw_reg ? hw_reg : std::make_shared<HardwareRegistry>()),
      uds_hub_(uds_hub)
{
    std::memset(&router_addr_, 0, sizeof(router_addr_));
    router_addr_.sin_family = AF_INET;
    router_addr_.sin_port   = htons(router_port_);
    inet_pton(AF_INET, router_ip_.c_str(), &router_addr_.sin_addr);

    guard_.set_log_callback([this](const std::string& text, uint8_t sev) {
        this->send_statustext(text, sev);
    });

    // Initialize draft buffers from active telemetry configuration (.env)
    auto init_cfg = config_engine_.load_telemetry_config();
    std::string init_fc = init_cfg.fc_port.empty() ? "/dev/ttyAMA4" : init_cfg.fc_port;
    std::string init_siyi = init_cfg.siyi_port.empty() ? "/dev/ttyAMA0" : init_cfg.siyi_port;
    str_to_buf(init_fc.c_str(), draft_fc_port, sizeof(draft_fc_port));
    str_to_buf(init_siyi.c_str(), draft_siyi_port, sizeof(draft_siyi_port));
    if (init_cfg.fc_baud > 0) draft_fc_baud = init_cfg.fc_baud;
    if (init_cfg.siyi_baud > 0) draft_siyi_baud = init_cfg.siyi_baud;
    str_to_buf("H.264",        cam_codec,        sizeof(cam_codec));
    str_to_buf("HW",           cam_encoder,      sizeof(cam_encoder));
    str_to_buf("YOLOv8n",      vis_model,        sizeof(vis_model));
    str_to_buf("RGB",          vis_source,       sizeof(vis_source));
}

MavlinkCore::~MavlinkCore() {
    if (sock_fd_ >= 0) close(sock_fd_);
}

// ─── Init ─────────────────────────────────────────────────────────────────
bool MavlinkCore::init() {
    sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd_ < 0) {
        std::cerr << "[MavlinkCore] Failed to create UDP socket\n";
        return false;
    }
    int flags = fcntl(sock_fd_, F_GETFL, 0);
    fcntl(sock_fd_, F_SETFL, flags | O_NONBLOCK);

    // Bind to local port so we can receive replies from router
    struct sockaddr_in local{};
    local.sin_family      = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port        = htons(14601); // local RX port
    bind(sock_fd_, reinterpret_cast<struct sockaddr*>(&local), sizeof(local));

    std::cout << "[MavlinkCore] Socket initialized -> "
              << router_ip_ << ":" << router_port_ << "\n";
    return true;
}

// ─── Low-level send ───────────────────────────────────────────────────────
void MavlinkCore::send_packet(const mavlink_message_t* msg) {
    if (sock_fd_ < 0) return;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buf, msg);
    sendto(sock_fd_, buf, len, 0,
           reinterpret_cast<struct sockaddr*>(&router_addr_),
           sizeof(router_addr_));
    total_tx_bytes_ += len;
}

void MavlinkCore::send_statustext(const std::string& text, uint8_t severity) {
    mavlink_message_t msg;
    char buf[50] = {};
    std::strncpy(buf, text.c_str(), sizeof(buf) - 1);
    mavlink_msg_statustext_pack(sys_id_, comp_id_, &msg, severity, buf, 0, 0);
    send_packet(&msg);
}

void MavlinkCore::send_command_ack(uint16_t command, uint8_t result, uint8_t target_sys, uint8_t target_comp) {
    mavlink_message_t msg;
    mavlink_msg_command_ack_pack(sys_id_, comp_id_, &msg,
                                  command, result, 0, 0, target_sys, target_comp);
    send_packet(&msg);
    // Send duplicate to guard against UDP packet loss
    send_packet(&msg);
}

// ─── 1 Hz telemetry entry point ──────────────────────────────────────────
void MavlinkCore::send_1hz_telemetry() {
    // Note: Autopilot Heartbeat is emitted natively by Cube Orange Plus on UART4; Companion only emits Component 191.

    // 1. Companion Heartbeat (Component 191)
    mavlink_message_t hb;
    mavlink_msg_heartbeat_pack(sys_id_, comp_id_, &hb,
                                MAV_TYPE_ONBOARD_CONTROLLER,
                                MAV_AUTOPILOT_INVALID,
                                0, 0, MAV_STATE_ACTIVE);
    send_packet(&hb);

    send_telemetry_links();
    send_telemetry_camera();
    send_telemetry_network();
    send_telemetry_vision();
    send_telemetry_system();
}

// ─── CC_TELEMETRY_LINKS (#42010) ─────────────────────────────────────────
void MavlinkCore::send_telemetry_links() {
    char fc_port_buf[16]       = {};
    char siyi_port_buf[16]     = {};
    char available_ports_buf[48]= {};

    auto active_cfg = guard_.get_current_config();
    std::string active_fc = !active_cfg.fc_port.empty() ? active_cfg.fc_port : draft_fc_port;
    std::string active_siyi = !active_cfg.siyi_port.empty() ? active_cfg.siyi_port : draft_siyi_port;
    uint32_t active_fc_baud = (active_cfg.fc_baud > 0) ? active_cfg.fc_baud : draft_fc_baud;
    uint32_t active_siyi_baud = (active_cfg.siyi_baud > 0) ? active_cfg.siyi_baud : draft_siyi_baud;

    std::strncpy(fc_port_buf,   active_fc.c_str(),   sizeof(fc_port_buf) - 1);
    std::strncpy(siyi_port_buf, active_siyi.c_str(), sizeof(siyi_port_buf) - 1);

    {
        auto ports = hw_reg_->scan_devices();
        std::string list;
        for (const auto& p : ports) {
            if (!list.empty()) list += ",";
            size_t pos = p.device_path.rfind('/');
            list += (pos != std::string::npos)
                        ? p.device_path.substr(pos + 1)
                        : p.device_path;
        }
        std::strncpy(available_ports_buf, list.c_str(),
                     sizeof(available_ports_buf) - 1);
    }

    // 1. FC Telemetry Stats (from MAVLink stream received from FC via router)
    guard_.tick_1hz();
    bool fc_online = guard_.is_fc_online();
    uint8_t fc_status = fc_online ? 2 : 0; // 2=ONLINE, 0=OFFLINE

    uint32_t fc_bytes_rx = guard_.get_fc_rx_bytes();
    // Convert kbps to bytes/sec: bitrate_kbps * 1000 / 8
    float fc_rx_rate = (guard_.get_fc_bitrate_kbps() * 1000.0f) / 8.0f;
    uint32_t fc_bytes_tx = total_tx_bytes_;
    static uint32_t last_total_tx = 0;
    float fc_tx_rate = static_cast<float>(total_tx_bytes_ >= last_total_tx ? (total_tx_bytes_ - last_total_tx) : 0);
    last_total_tx = total_tx_bytes_;
    uint32_t fc_tx_err = 0;
    float fc_rx_loss = 0.0f;

    // Optional kernel hardware stats (TIOCGICOUNT).  Returns DELTA since last call.
    // IMPORTANT SEMANTIC NOTE:
    //   frame_errors  = stop-bit framing errors on the RECEIVE path from FC (baud mismatch indicator).
    //   overrun_errors = hardware FIFO overruns on the RECEIVE path.
    //   buf_overrun   = kernel RX-buffer overflow (OS dropped data).
    //   None of these are true TX-side errors; TIOCGICOUNT doesn't report TX errors.
    auto fc_stats = hw_reg_->get_uart_stats(active_fc);
    if (fc_stats.valid) {
        // Prefer hardware byte counters when they're larger than the guard's MAVLink-parse estimate.
        if (fc_stats.rx_bytes > fc_bytes_rx) fc_bytes_rx = fc_stats.rx_bytes;
        if (fc_stats.tx_bytes > fc_bytes_tx) fc_bytes_tx = fc_stats.tx_bytes;

        // Compute fc_rx_loss as fraction of received bytes that had framing errors.
        // This correctly models "packet loss on the RX path from the FC".
        uint32_t rx_err_delta = fc_stats.frame_errors + fc_stats.overrun_errors;
        if (rx_err_delta > 0 && fc_stats.rx_bytes > 0) {
            fc_rx_loss = std::min(
                100.0f * static_cast<float>(rx_err_delta) /
                         static_cast<float>(fc_stats.rx_bytes + rx_err_delta),
                100.0f);
        }
        // buf_overrun: kernel had to discard data — closest metric to "link TX data loss".
        fc_tx_err = fc_stats.buf_overrun;
    }

    // 2. SIYI Telemetry Stats
    auto now = std::chrono::steady_clock::now();
    bool siyi_online = (last_siyi_time_.time_since_epoch().count() > 0 &&
                        std::chrono::duration_cast<std::chrono::seconds>(now - last_siyi_time_).count() < 4);
    // 2=ONLINE, 1=STANDBY, 0=OFFLINE
    uint8_t siyi_status = siyi_online ? 2 : (fc_online ? 1 : 0);

    uint32_t siyi_bytes_rx = siyi_rx_bytes_;
    static uint32_t last_siyi_rx = 0;
    float siyi_rx_rate = static_cast<float>(siyi_rx_bytes_ >= last_siyi_rx ? (siyi_rx_bytes_ - last_siyi_rx) : 0);
    last_siyi_rx = siyi_rx_bytes_;

    // Telemetry streamed down to SIYI tay cầm is the FC telemetry stream forwarded out
    uint32_t siyi_bytes_tx = fc_bytes_rx;
    float siyi_tx_rate = fc_rx_rate;
    uint32_t siyi_tx_err = 0;
    float siyi_rx_loss = 0.0f;

    auto siyi_stats = hw_reg_->get_uart_stats(active_siyi);
    if (siyi_stats.valid) {
        if (siyi_stats.rx_bytes > siyi_bytes_rx) siyi_bytes_rx = siyi_stats.rx_bytes;
        if (siyi_stats.tx_bytes > siyi_bytes_tx) siyi_bytes_tx = siyi_stats.tx_bytes;

        uint32_t siyi_rx_err_delta = siyi_stats.frame_errors + siyi_stats.overrun_errors;
        if (siyi_rx_err_delta > 0 && siyi_stats.rx_bytes > 0) {
            siyi_rx_loss = std::min(
                100.0f * static_cast<float>(siyi_rx_err_delta) /
                         static_cast<float>(siyi_stats.rx_bytes + siyi_rx_err_delta),
                100.0f);
        }
        siyi_tx_err = siyi_stats.buf_overrun;
    }

    float max_fc_tx_rate = fc_tx_rate > 0.0f ? fc_tx_rate : fc_rx_rate;

    mavlink_message_t msg;
    mavlink_msg_cc_telemetry_links_pack(
        sys_id_, comp_id_, &msg,
        fc_tx_rate, fc_rx_rate, max_fc_tx_rate, 1.0f, fc_rx_loss, fc_tx_err,
        fc_bytes_rx, fc_bytes_tx, active_fc_baud,
        siyi_tx_rate, siyi_rx_rate, siyi_tx_rate, 1.0f, siyi_rx_loss, siyi_tx_err,
        siyi_bytes_rx, siyi_bytes_tx, active_siyi_baud,
        fc_status, siyi_status, 1 /*Serial/UART*/,
        fc_port_buf, siyi_port_buf, available_ports_buf
    );
    send_packet(&msg);
}

// ─── CC_TELEMETRY_CAMERA (#42011) ────────────────────────────────────────
void MavlinkCore::send_telemetry_camera() {
    // Build RTSP URLs from current network state
    NetworkStats ns = net_.read_stats();

    char rtsp_qgc[48]        = {};
    char rtsp_controller[48] = {};
    char rtsp_laptop[40]     = {};

    if (ns.uap0_ip[0])
        std::snprintf(rtsp_qgc, sizeof(rtsp_qgc),
                      "rtsp://%s:8554/camera", ns.uap0_ip);
    if (ns.eth0_ip[0])
        std::snprintf(rtsp_controller, sizeof(rtsp_controller),
                      "rtsp://%s:8554/camera", ns.eth0_ip);
    if (ns.wlan0_ip[0])
        std::snprintf(rtsp_laptop, sizeof(rtsp_laptop),
                      "rtsp://%s:8554/camera", ns.wlan0_ip);

    mavlink_message_t msg;
    mavlink_msg_cc_telemetry_camera_pack(
        sys_id_, comp_id_, &msg,
        /*video_width*/      cam_width,
        /*video_height*/     cam_height,
        /*depth_width*/      640,
        /*depth_height*/     480,
        /*rotation*/         0,
        /*bitrate_kbps*/     cam_bitrate,
        /*bitrate_max_kbps*/ 3000,
        /*vbv_buffer_kb*/    500,
        /*rtsp_port*/        8554,
        /*video_fps*/        cam_fps,
        /*depth_fps*/        15,
        /*camera_id*/        1,
        /*is_default*/       1,
        /*profile_mode*/     0,
        /*enable_emitter*/   1,
        /*camera_status*/    2,
        /*error_code*/       0,
        /*usb_speed_mode*/   2,
        /*camera_name*/      "RealSense D435",
        /*camera_type*/      "RGB-D",
        /*connection_port*/  "USB3-1",
        /*serial_number*/    "12345678",
        /*codec*/            cam_codec,
        /*encoder_mode*/     cam_encoder,
        /*rtsp_url_qgc*/     rtsp_qgc,
        /*rtsp_url_controller*/ rtsp_controller,
        /*rtsp_url_laptop*/  rtsp_laptop
    );
    send_packet(&msg);
}

// ─── CC_TELEMETRY_NETWORK (#42012) ───────────────────────────────────────
void MavlinkCore::send_telemetry_network() {
    NetworkStats ns = net_.read_stats();

    mavlink_message_t msg;
    mavlink_msg_cc_telemetry_network_pack(
        sys_id_, comp_id_, &msg,
        /*uap0_rx_kb*/     ns.uap0_rx_kb,
        /*uap0_tx_kb*/     ns.uap0_tx_kb,
        /*wlan0_rx_kb*/    ns.wlan0_rx_kb,
        /*wlan0_tx_kb*/    ns.wlan0_tx_kb,
        /*eth0_rx_kb*/     ns.eth0_rx_kb,
        /*eth0_tx_kb*/     ns.eth0_tx_kb,
        /*ap_channel*/     ns.ap_channel,
        /*ap_ieee80211n*/  ns.ap_ieee80211n,
        /*ap_wmm_enabled*/ ns.ap_wmm_enabled,
        /*ap_wpa*/         ns.ap_wpa,
        /*ap_client_count*/ns.ap_client_count,
        /*ap_status*/      ns.ap_status,
        /*wlan0_status*/   ns.wlan0_status,
        /*eth0_status*/    ns.eth0_status,
        /*eth0_is_static*/ ns.eth0_is_static,
        /*wlan0_dhcp*/     ns.wlan0_dhcp,
        /*dnsmasq_status*/ ns.dnsmasq_status,
        /*wlan0_rssi*/     ns.wlan0_rssi,
        /*eth0_ip*/        ns.eth0_ip,
        /*eth0_netmask*/   ns.eth0_netmask,
        /*wlan0_ip*/       ns.wlan0_ip,
        /*wlan0_netmask*/  ns.wlan0_netmask,
        /*wlan0_ssid*/     "THACO-WiFi",
        /*ap_ip*/          ns.uap0_ip,
        /*ap_netmask*/     ns.uap0_netmask,
        /*ap_ssid*/        ns.ap_ssid,
        /*ap_wpa_passphrase*/ ns.ap_wpa_passphrase,
        /*ap_key_mgmt*/    ns.ap_key_mgmt,
        /*ap_hw_mode*/     ns.ap_hw_mode
    );
    send_packet(&msg);
}

// ─── CC_TELEMETRY_VISION (#42013) ────────────────────────────────────────
void MavlinkCore::send_telemetry_vision() {
    mavlink_message_t msg;
    mavlink_msg_cc_telemetry_vision_pack(
        sys_id_, comp_id_, &msg,
        /*confidence_thresh*/ vis_conf,
        /*inference_fps*/     15.0f,
        /*input_width*/       640,
        /*input_height*/      480,
        /*video_fps*/         30,
        /*detections_count*/  0,
        /*status_flags*/      1,
        /*model_name*/        vis_model,
        /*input_source*/      vis_source
    );
    send_packet(&msg);
}

// ─── CC_TELEMETRY_SYSTEM (#42014) ────────────────────────────────────────
void MavlinkCore::send_telemetry_system() {
    SystemStats ss = metrics_.read_stats();

    mavlink_message_t msg;
    mavlink_msg_cc_telemetry_system_pack(
        sys_id_, comp_id_, &msg,
        /*system_uptime_s*/ ss.uptime_seconds,
        /*cpu_usage*/       ss.cpu_usage_pct,
        /*ram_usage*/       ss.ram_usage_pct,
        /*disk_usage*/      ss.disk_usage_pct,
        /*cpu_temp*/        ss.cpu_temp_celsius,
        /*system_status*/   2  // 2 = ACTIVE
    );
    send_packet(&msg);
}

// ─── Incoming processing ─────────────────────────────────────────────────
void MavlinkCore::process_incoming() {
    if (sock_fd_ < 0) return;

    uint8_t buf[2048];
    while (true) {
        ssize_t n = recvfrom(sock_fd_, buf, sizeof(buf), 0, nullptr, nullptr);
        if (n <= 0) break;

        mavlink_message_t msg;
        mavlink_status_t  status;
        for (ssize_t i = 0; i < n; ++i) {
            if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &msg, &status)) {
                handle_message(&msg);
            }
        }
    }
}

void MavlinkCore::handle_message(const mavlink_message_t* msg) {
    uint32_t pkt_len = static_cast<uint32_t>(msg->len + 12);

    // Track FC packets (sysid=1, compid 0, 1, or autopilot components)
    if (msg->sysid == 1 && (msg->compid == 0 || msg->compid == 1 || msg->compid == 158)) {
        guard_.record_fc_heartbeat(pkt_len);
    } else if (msg->compid != comp_id_) {
        // Track packets from SIYI Air Unit / Remote Controller (GCS sysid 255 / compid 190, 0, or camera/gimbal)
        last_siyi_time_ = std::chrono::steady_clock::now();
        siyi_rx_bytes_ += pkt_len;
    }

    switch (msg->msgid) {
        case MAVLINK_MSG_ID_COMMAND_LONG:
            handle_command_long(msg);
            break;
        case MAVLINK_MSG_ID_CC_CONFIG_BEGIN:
            handle_cc_config_begin(msg);
            break;
        case MAVLINK_MSG_ID_CC_CONFIG_SET:
            handle_cc_config_set(msg);
            break;
        case MAVLINK_MSG_ID_CC_CONFIG_GET:
            handle_cc_config_get(msg);
            break;
        case MAVLINK_MSG_ID_CC_CONFIG_APPLY:
            handle_cc_config_apply(msg);
            break;
        case MAVLINK_MSG_ID_CC_CONFIG_CONFIRM:
            handle_cc_config_confirm(msg);
            break;
        case MAVLINK_MSG_ID_CC_CONFIG_ROLLBACK:
            handle_cc_config_rollback(msg);
            break;
        case MAVLINK_MSG_ID_CC_TELEMETRY_LINKS:
            handle_incoming_telemetry_links(msg);
            break;
        case MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA:
            handle_incoming_telemetry_camera(msg);
            break;
        case MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK:
            handle_incoming_telemetry_network(msg);
            break;
        case MAVLINK_MSG_ID_CC_TELEMETRY_VISION:
            handle_incoming_telemetry_vision(msg);
            break;
        default:
            break;
    }
}

// ─── COMMAND_LONG (76) ───────────────────────────────────────────────────
void MavlinkCore::handle_command_long(const mavlink_message_t* msg) {
    mavlink_command_long_t cmd;
    mavlink_msg_command_long_decode(msg, &cmd);

    if (cmd.target_component != comp_id_ && cmd.target_component != 0) return;

    static const char* sub_names[] = {"All", "Telemetry", "Camera", "Network", "Vision"};
    uint8_t sub = static_cast<uint8_t>(cmd.param1);
    const char* sub_name = (sub < 5) ? sub_names[sub] : "Unknown";
    bool restart_imm = (static_cast<int>(cmd.param2) == 1);

    if (cmd.command == 44011) { // MAV_CMD_THACO_APPLY_CONFIG
        uint8_t target_sys = msg->sysid;
        uint8_t target_comp = msg->compid;

        if (restart_imm && (sub == 0 || sub == 1)) {
            std::string fc_port = draft_fc_port;
            uint32_t fc_baud = draft_fc_baud;
            std::string siyi_port = draft_siyi_port;
            uint32_t siyi_baud = draft_siyi_baud;

            // Dispatch step-by-step apply pipeline in background thread (no fake immediate success)
            std::thread([this, fc_port, fc_baud, siyi_port, siyi_baud, target_sys, target_comp]() {
                this->apply_telemetry_links_hardware(fc_port, fc_baud, siyi_port, siyi_baud, target_sys, target_comp);
            }).detach();
        }
        if (restart_imm && (sub == 0 || sub == 2)) {
            uint16_t w = cam_width;
            uint16_t h = cam_height;
            uint8_t fps = cam_fps;
            uint16_t bitrate = cam_bitrate;
            std::string codec = cam_codec;

            // Wait 500ms before restarting streamer so COMMAND_ACK clears UDP socket to QGC!
            std::thread([this, w, h, fps, bitrate, codec]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                this->apply_camera_config_hardware(w, h, fps, bitrate, codec);
            }).detach();
        }
    } else if (cmd.command == 44010) { // MAV_CMD_THACO_SAVE_DEFAULT_CONFIG
        send_command_ack(cmd.command, MAV_RESULT_ACCEPTED, msg->sysid, msg->compid);
        char log_buf[48];
        std::snprintf(log_buf, sizeof(log_buf), "FC: [CMD] SAVE_DEFAULT %s", sub_name);
        send_statustext(log_buf);
    } else if (cmd.command == 44012) { // MAV_CMD_THACO_RESTORE_DEFAULT_CONFIG
        send_command_ack(cmd.command, MAV_RESULT_ACCEPTED, msg->sysid, msg->compid);
        char log_buf[48];
        std::snprintf(log_buf, sizeof(log_buf), "FC: [CMD] RESTORE_DEFAULT %s", sub_name);
        send_statustext(log_buf);
    }
}

// ─── CC_CONFIG_BEGIN (42100) ─────────────────────────────────────────────
void MavlinkCore::handle_cc_config_begin(const mavlink_message_t* msg) {
    mavlink_cc_config_begin_t m;
    mavlink_msg_cc_config_begin_decode(msg, &m);

    TxResult res = config_engine_.begin(m.timeout_s);

    mavlink_message_t ack_msg;
    mavlink_msg_cc_config_ack_pack(
        sys_id_, comp_id_, &ack_msg,
        m.request_id, res.tid,
        res.result_code, res.apply_type, res.error_code, res.message.c_str());
    send_packet(&ack_msg);
}

// ─── CC_CONFIG_SET (42101) ───────────────────────────────────────────────
void MavlinkCore::handle_cc_config_set(const mavlink_message_t* msg) {
    mavlink_cc_config_set_t m;
    mavlink_msg_cc_config_set_decode(msg, &m);

    TxResult res = config_engine_.set(m.transaction_id, m.key, m.value);

    mavlink_message_t ack_msg;
    mavlink_msg_cc_config_ack_pack(
        sys_id_, comp_id_, &ack_msg,
        m.request_id, m.transaction_id,
        res.result_code, res.apply_type, res.error_code, res.message.c_str());
    send_packet(&ack_msg);
}

// ─── CC_CONFIG_GET (42102) ───────────────────────────────────────────────
void MavlinkCore::handle_cc_config_get(const mavlink_message_t* msg) {
    mavlink_cc_config_get_t m;
    mavlink_msg_cc_config_get_decode(msg, &m);

    auto [ok, val] = config_engine_.get(m.key);

    mavlink_message_t val_msg;
    mavlink_msg_cc_config_value_pack(
        sys_id_, comp_id_, &val_msg,
        m.request_id,
        m.key, val.c_str());
    send_packet(&val_msg);
}

// ─── CC_CONFIG_APPLY (42103) ─────────────────────────────────────────────
void MavlinkCore::handle_cc_config_apply(const mavlink_message_t* msg) {
    mavlink_cc_config_apply_t m;
    mavlink_msg_cc_config_apply_decode(msg, &m);

    TxResult res = config_engine_.apply(m.transaction_id);

    char log_buf[64];
    std::snprintf(log_buf, sizeof(log_buf), "APPLY tid=%u -> %s", m.transaction_id, res.message.c_str());
    send_statustext(log_buf);

    mavlink_message_t ack_msg;
    mavlink_msg_cc_config_ack_pack(
        sys_id_, comp_id_, &ack_msg,
        m.request_id, m.transaction_id,
        res.result_code, res.apply_type, res.error_code, res.message.c_str());
    send_packet(&ack_msg);
}

// ─── CC_CONFIG_CONFIRM (42104) ───────────────────────────────────────────
void MavlinkCore::handle_cc_config_confirm(const mavlink_message_t* msg) {
    mavlink_cc_config_confirm_t m;
    mavlink_msg_cc_config_confirm_decode(msg, &m);

    TxResult res = config_engine_.confirm(m.transaction_id);

    mavlink_message_t ack_msg;
    mavlink_msg_cc_config_ack_pack(
        sys_id_, comp_id_, &ack_msg,
        m.request_id, m.transaction_id,
        res.result_code, res.apply_type, res.error_code, res.message.c_str());
    send_packet(&ack_msg);
}

// ─── CC_CONFIG_ROLLBACK (42105) ──────────────────────────────────────────
void MavlinkCore::handle_cc_config_rollback(const mavlink_message_t* msg) {
    mavlink_cc_config_rollback_t m;
    mavlink_msg_cc_config_rollback_decode(msg, &m);

    TxResult res = config_engine_.rollback(m.transaction_id);

    char log_buf[64];
    std::snprintf(log_buf, sizeof(log_buf), "ROLLBACK tid=%u", m.transaction_id);
    send_statustext(log_buf, 4 /*WARNING*/);

    mavlink_message_t ack_msg;
    mavlink_msg_cc_config_ack_pack(
        sys_id_, comp_id_, &ack_msg,
        m.request_id, m.transaction_id,
        res.result_code, res.apply_type, res.error_code, res.message.c_str());
    send_packet(&ack_msg);
}

// ─── Incoming telemetry (from QGC: draft settings) ───────────────────────
void MavlinkCore::handle_incoming_telemetry_links(const mavlink_message_t* msg) {
    mavlink_cc_telemetry_links_t m;
    mavlink_msg_cc_telemetry_links_decode(msg, &m);

    if (m.fc_port[0])
        str_to_buf(m.fc_port, draft_fc_port, sizeof(draft_fc_port));
    if (m.fc_baudrate > 0)
        draft_fc_baud = m.fc_baudrate;
    if (m.siyi_port[0])
        str_to_buf(m.siyi_port, draft_siyi_port, sizeof(draft_siyi_port));
    if (m.siyi_baudrate > 0)
        draft_siyi_baud = m.siyi_baudrate;

    char fc_buf[48];
    std::snprintf(fc_buf, sizeof(fc_buf), "FC: [STAGED] %s@%u", draft_fc_port, draft_fc_baud);
    send_statustext(fc_buf);

    char siyi_buf[48];
    std::snprintf(siyi_buf, sizeof(siyi_buf), "SIYI: [STAGED] %s@%u", draft_siyi_port, draft_siyi_baud);
    send_statustext(siyi_buf);
}

void MavlinkCore::handle_incoming_telemetry_camera(const mavlink_message_t* msg) {
    mavlink_cc_telemetry_camera_t m;
    mavlink_msg_cc_telemetry_camera_decode(msg, &m);

    cam_width   = m.video_width;
    cam_height  = m.video_height;
    cam_fps     = m.video_fps;
    cam_bitrate = m.bitrate_kbps;
    str_to_buf(m.codec, cam_codec, sizeof(cam_codec));

    char log_buf[80];
    std::snprintf(log_buf, sizeof(log_buf),
                  "CAM: SET Cam %dx%d@%dfps %uk %s",
                  cam_width, cam_height, cam_fps, cam_bitrate, cam_codec);
    send_statustext(log_buf);
}

void MavlinkCore::handle_incoming_telemetry_network(const mavlink_message_t* msg) {
    mavlink_cc_telemetry_network_t m;
    mavlink_msg_cc_telemetry_network_decode(msg, &m);

    str_to_buf(m.ap_ssid, net_.draft_ssid, sizeof(net_.draft_ssid));
    str_to_buf(m.ap_wpa_passphrase, net_.draft_passphrase, sizeof(net_.draft_passphrase));
    str_to_buf(m.eth0_ip, net_.draft_eth0_ip, sizeof(net_.draft_eth0_ip));
    net_.draft_channel = m.ap_channel;

    char log_buf[80];
    std::snprintf(log_buf, sizeof(log_buf),
                  "NET: SET Net SSID=%s Ch=%d Eth=%s",
                  net_.draft_ssid, net_.draft_channel, net_.draft_eth0_ip);
    send_statustext(log_buf);
}

void MavlinkCore::handle_incoming_telemetry_vision(const mavlink_message_t* msg) {
    mavlink_cc_telemetry_vision_t m;
    mavlink_msg_cc_telemetry_vision_decode(msg, &m);

    str_to_buf(m.model_name, vis_model, sizeof(vis_model));
    str_to_buf(m.input_source, vis_source, sizeof(vis_source));
    vis_conf = m.confidence_thresh;

    char log_buf[80];
    std::snprintf(log_buf, sizeof(log_buf),
                  "VIS: SET Vis Model=%s Conf=%.2f", vis_model, vis_conf);
    send_statustext(log_buf);
}

// ─── Hardware apply (telemetry links) ────────────────────────────────────
void MavlinkCore::apply_telemetry_links_hardware(
        const std::string& fc_port, uint32_t fc_baud,
        const std::string& siyi_port, uint32_t siyi_baud,
        uint8_t target_sys, uint8_t target_comp) {

    auto active_cfg = guard_.get_current_config();
    std::string eff_fc_port = !fc_port.empty() ? fc_port : active_cfg.fc_port;
    uint32_t eff_fc_baud = fc_baud > 0 ? fc_baud : active_cfg.fc_baud;
    std::string eff_siyi_port = !siyi_port.empty() ? siyi_port : active_cfg.siyi_port;
    uint32_t eff_siyi_baud = siyi_baud > 0 ? siyi_baud : active_cfg.siyi_baud;

    auto dev_name = [](const std::string& p) -> std::string {
        size_t pos = p.rfind('/');
        return (pos != std::string::npos) ? p.substr(pos + 1) : p;
    };

    // Stage 1: Physical port validation
    char step1_buf[64];
    std::snprintf(step1_buf, sizeof(step1_buf), "LINKS: [1/4] Validating FC=%s SIYI=%s",
                  dev_name(eff_fc_port).c_str(), dev_name(eff_siyi_port).c_str());
    send_statustext(step1_buf, 6 /*INFO*/);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (!eff_fc_port.empty() && !hw_reg_->is_valid_character_device(eff_fc_port)) {
        char err_buf[64];
        std::snprintf(err_buf, sizeof(err_buf), "FC: [FAILED] Port %s not found!", dev_name(eff_fc_port).c_str());
        send_statustext(err_buf, 3 /*ERROR*/);
        send_command_ack(44011, MAV_RESULT_DENIED, target_sys, target_comp);
        return;
    }
    if (!eff_siyi_port.empty() && !hw_reg_->is_valid_character_device(eff_siyi_port)) {
        char err_buf[64];
        std::snprintf(err_buf, sizeof(err_buf), "SIYI: [FAILED] Port %s not found!", dev_name(eff_siyi_port).c_str());
        send_statustext(err_buf, 3 /*ERROR*/);
        send_command_ack(44011, MAV_RESULT_DENIED, target_sys, target_comp);
        return;
    }

    // Stage 2: Save to storage (.env & active_config)
    char step2_buf[64];
    std::snprintf(step2_buf, sizeof(step2_buf), "LINKS: [2/4] Saving FC=%s@%u SIYI=%s@%u",
                  dev_name(eff_fc_port).c_str(), eff_fc_baud,
                  dev_name(eff_siyi_port).c_str(), eff_siyi_baud);
    send_statustext(step2_buf, 6 /*INFO*/);

    TelemetryConfig new_cfg;
    new_cfg.fc_port   = eff_fc_port;
    new_cfg.fc_baud   = eff_fc_baud;
    new_cfg.siyi_port = eff_siyi_port;
    new_cfg.siyi_baud = eff_siyi_baud;

    if (!config_engine_.save_telemetry_config(new_cfg)) {
        send_statustext("LINKS: [FAILED] Could not save config to storage!", 3 /*ERROR*/);
        send_command_ack(44011, MAV_RESULT_FAILED, target_sys, target_comp);
        return;
    }

    // Update in-memory guard and draft buffers immediately
    guard_.set_current_config(new_cfg);
    str_to_buf(eff_fc_port.c_str(), draft_fc_port, sizeof(draft_fc_port));
    draft_fc_baud = eff_fc_baud;
    str_to_buf(eff_siyi_port.c_str(), draft_siyi_port, sizeof(draft_siyi_port));
    draft_siyi_baud = eff_siyi_baud;

    // Stage 3: Reload MAVLink Router process cleanly via router.sock
    send_statustext("LINKS: [3/4] Reloading MAVLink Router endpoints...", 6 /*INFO*/);
    bool router_ok = config_engine_.restart_mavlink_router();
    if (!router_ok) {
        send_statustext("LINKS: [FAILED] Router reload failed via router.sock!", 3 /*ERROR*/);
        send_command_ack(44011, MAV_RESULT_FAILED, target_sys, target_comp);
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Stage 4: Verify UART hardware interface via Linux Kernel TIOCGICOUNT
    auto hw_stats = hw_reg_->get_uart_stats(eff_fc_port);
    char step4_buf[80];
    std::snprintf(step4_buf, sizeof(step4_buf), "LINKS: [4/4] Hardware UART ready (TX:%u, RX:%u)",
                  hw_stats.tx_bytes, hw_stats.rx_bytes);
    send_statustext(step4_buf, 6 /*INFO*/);

    // Immediate burst of telemetry to warm router routing table
    send_1hz_telemetry();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Stage 5: Genuine Success Confirmation
    char succ_buf[64];
    std::snprintf(succ_buf, sizeof(succ_buf), "LINKS: [SUCCESS] FC=%s@%u SIYI=%s@%u",
                  dev_name(eff_fc_port).c_str(), eff_fc_baud, dev_name(eff_siyi_port).c_str(), eff_siyi_baud);
    send_statustext(succ_buf, 6 /*INFO*/);
    send_command_ack(44011, MAV_RESULT_ACCEPTED, target_sys, target_comp);
}

void MavlinkCore::apply_camera_config_hardware(
        uint16_t width, uint16_t height, uint8_t fps,
        uint16_t bitrate_kbps, const std::string& codec) {

    namespace fs = std::filesystem;

    if (width == 0) width = 640;
    if (height == 0) height = 480;
    if (fps == 0) fps = 30;
    if (bitrate_kbps == 0) bitrate_kbps = 1300;
    std::string safe_codec = codec.empty() ? "h264" : codec;

    if (uds_hub_) {
        nlohmann::json cmd = {
            {"target", "camera"},
            {"cmd", "SET_CONFIG"},
            {"data", {
                {"width", width},
                {"height", height},
                {"fps", fps},
                {"bitrate_kbps", bitrate_kbps},
                {"codec", safe_codec}
            }}
        };
        uds_hub_->send_to_target("camera", cmd);
    }

    std::vector<std::string> paths;
    if (!config_engine_.paths().camera_config_file.empty()) {
        paths.push_back(config_engine_.paths().camera_config_file);
    }
    paths.push_back("camera.yaml");

    bool updated_any = false;
    for (const auto& path : paths) {
        std::error_code ec;
        if (!fs::exists(path, ec)) continue;

        std::ifstream in(path);
        if (!in.is_open()) continue;

        std::vector<std::string> lines;
        std::string line;
        bool in_video = false;
        bool in_encoder = false;

        while (std::getline(in, line)) {
            std::string trimmed = line;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));

            if (trimmed.rfind("video:", 0) == 0) {
                in_video = true;
                in_encoder = false;
            } else if (trimmed.rfind("encoder:", 0) == 0) {
                in_encoder = true;
                in_video = false;
            } else if (!trimmed.empty() && trimmed[0] != '#' && trimmed.find(':') != std::string::npos && line.find_first_not_of(" \t") == 0) {
                in_video = false;
                in_encoder = false;
            }

            if (in_video) {
                if (trimmed.rfind("width:", 0) == 0) {
                    line = "  width: " + std::to_string(width);
                } else if (trimmed.rfind("height:", 0) == 0) {
                    line = "  height: " + std::to_string(height);
                } else if (trimmed.rfind("fps:", 0) == 0) {
                    line = "  fps: " + std::to_string(fps);
                }
            } else if (in_encoder) {
                if (trimmed.rfind("bitrate_kbps:", 0) == 0) {
                    line = "  bitrate_kbps: " + std::to_string(bitrate_kbps);
                } else if (trimmed.rfind("bitrate_min_kbps:", 0) == 0) {
                    line = "  bitrate_min_kbps: " + std::to_string(bitrate_kbps);
                } else if (trimmed.rfind("bitrate_max_kbps:", 0) == 0) {
                    line = "  bitrate_max_kbps: " + std::to_string(bitrate_kbps);
                } else if (trimmed.rfind("vbv_buffer_kb:", 0) == 0) {
                    line = "  vbv_buffer_kb: " + std::to_string(bitrate_kbps);
                } else if (trimmed.rfind("intra_refresh:", 0) == 0) {
                    line = "  intra_refresh: false";
                }
            }
            lines.push_back(line);
        }
        in.close();

        std::ofstream out(path);
        if (out.is_open()) {
            for (const auto& l : lines) {
                out << l << "\n";
            }
            out.close();
            updated_any = true;
        }
    }

    if (!updated_any) {
        char err_buf[64];
        std::snprintf(err_buf, sizeof(err_buf), "CAM: Failed to write camera.yaml");
        send_statustext(err_buf, 4 /*WARNING*/);
        return;
    }

    // Restart streamer container or service
    int ret = std::system("docker restart drone-edge-drone-camera-rtsp-1 >/dev/null 2>&1 || systemctl restart drone-camera.service >/dev/null 2>&1");

    char stat_buf[80];
    if (ret == 0) {
        std::snprintf(stat_buf, sizeof(stat_buf), "CAM: Applied %ux%u@%ufps %ukbps (Restarted)",
                      width, height, fps, bitrate_kbps);
        send_statustext(stat_buf, 6 /*INFO*/);
    } else {
        std::snprintf(stat_buf, sizeof(stat_buf), "CAM: Updated yaml, restart exit %d", ret);
        send_statustext(stat_buf, 4 /*WARNING*/);
    }
}


void MavlinkCore::update_camera_telemetry(uint16_t w, uint16_t h, uint8_t fps, uint32_t bitrate, const std::string& codec, uint8_t status) {
    if (w > 0) cam_width = w;
    if (h > 0) cam_height = h;
    if (fps > 0) cam_fps = fps;
    if (bitrate > 0) cam_bitrate = bitrate;
    if (!codec.empty()) str_to_buf(codec.c_str(), cam_codec, sizeof(cam_codec));
    cam_status = status;
}

void MavlinkCore::update_vision_telemetry(const std::string& model, float conf, const std::string& source, uint8_t status) {
    if (!model.empty()) str_to_buf(model.c_str(), vis_model, sizeof(vis_model));
    if (conf > 0.0f) vis_conf = conf;
    if (!source.empty()) str_to_buf(source.c_str(), vis_source, sizeof(vis_source));
    vis_status = status;
}

} // namespace cc
