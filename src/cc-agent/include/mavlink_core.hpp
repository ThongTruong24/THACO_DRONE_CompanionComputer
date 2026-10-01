#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <netinet/in.h>
#include <cstring>

#include "thaco_common/mavlink.h"
#include "rollback_guard.hpp"
#include "system_metrics.hpp"
#include "network_monitor.hpp"
#include "config_engine.hpp"
#include "hardware_registry.hpp"
#include "uds_hub_server.hpp"

namespace cc {

class MavlinkCore {
public:
    MavlinkCore(const std::string& router_ip, int router_port,
                RollbackGuard& guard,
                SystemMetricsCollector& metrics,
                NetworkMonitor& net,
                ConfigEngine& config_engine,
                std::shared_ptr<HardwareRegistry> hw_reg,
                std::shared_ptr<UdsHubServer> uds_hub = nullptr);
    ~MavlinkCore();

    bool init();
    void process_incoming();
    void send_1hz_telemetry();
    void send_statustext(const std::string& text, uint8_t severity = 6);
    void send_command_ack(uint16_t command, uint8_t result, uint8_t target_sys = 0, uint8_t target_comp = 0);

    // Callbacks for live telemetry updates from containers via UDS Hub
    void update_camera_telemetry(uint16_t w, uint16_t h, uint8_t fps, uint32_t bitrate, const std::string& codec, uint8_t status);
    void update_vision_telemetry(const std::string& model, float conf, const std::string& source, uint8_t status);

private:
    void send_packet(const mavlink_message_t* msg);
    void handle_message(const mavlink_message_t* msg);

    // Telemetry senders
    void send_telemetry_links();
    void send_telemetry_camera();
    void send_telemetry_network();
    void send_telemetry_vision();
    void send_telemetry_system();

    // Config message handlers (direct C++ native execution)
    void handle_cc_config_begin(const mavlink_message_t* msg);
    void handle_cc_config_set(const mavlink_message_t* msg);
    void handle_cc_config_get(const mavlink_message_t* msg);
    void handle_cc_config_apply(const mavlink_message_t* msg);
    void handle_cc_config_confirm(const mavlink_message_t* msg);
    void handle_cc_config_rollback(const mavlink_message_t* msg);
    void handle_command_long(const mavlink_message_t* msg);
    void handle_incoming_telemetry_links(const mavlink_message_t* msg);
    void handle_incoming_telemetry_camera(const mavlink_message_t* msg);
    void handle_incoming_telemetry_network(const mavlink_message_t* msg);
    void handle_incoming_telemetry_vision(const mavlink_message_t* msg);

    // Hardware apply
    void apply_telemetry_links_hardware(const std::string& fc_port, uint32_t fc_baud,
                                         const std::string& siyi_port, uint32_t siyi_baud,
                                         uint8_t target_sys = 255, uint8_t target_comp = 190);
    void apply_camera_config_hardware(uint16_t width, uint16_t height, uint8_t fps,
                                      uint16_t bitrate_kbps, const std::string& codec);

    std::string router_ip_;
    int router_port_;
    int sock_fd_ = -1;
    struct sockaddr_in router_addr_;

    RollbackGuard&         guard_;
    SystemMetricsCollector& metrics_;
    NetworkMonitor&        net_;
    ConfigEngine&          config_engine_;
    std::shared_ptr<HardwareRegistry> hw_reg_;
    std::shared_ptr<UdsHubServer>     uds_hub_;

    uint8_t sys_id_  = 1;
    uint8_t comp_id_ = 191;

    uint32_t total_tx_bytes_ = 0;

    // Live / Draft camera settings
    uint16_t cam_width   = 1920;
    uint16_t cam_height  = 1080;
    uint8_t  cam_fps     = 30;
    uint32_t cam_bitrate = 2000;
    char     cam_codec[6]= {"H.264"};
    char     cam_encoder[6] = {"HW"};
    uint8_t  cam_status  = 2; // 2=ONLINE

    // Live / Draft vision settings
    char    vis_model[32]  = {"YOLOv8n"};
    char    vis_source[16] = {"RGB"};
    float   vis_conf       = 0.5f;
    uint8_t vis_status     = 2; // 2=ONLINE

    // Draft link settings (staged by QGC, pending APPLY_CONFIG)
    char    draft_fc_port[16]   = {"/dev/ttyAMA4"};
    uint32_t draft_fc_baud      = 921600;
    char    draft_siyi_port[16] = {"/dev/ttyAMA0"};
    uint32_t draft_siyi_baud    = 115200;

    // Real SIYI Telemetry Tracking
    std::chrono::steady_clock::time_point last_siyi_time_{};
    uint32_t siyi_rx_bytes_{0};
    uint32_t last_siyi_rx_bytes_{0};
    float    max_siyi_tx_rate_{0.0f};
};

} // namespace cc
