#include "app_paths.hpp"
#include "hardware_registry.hpp"
#include "uds_hub_server.hpp"
#include "system_metrics.hpp"
#include "config_engine.hpp"
#include "rollback_guard.hpp"
#include "network_monitor.hpp"
#include "mavlink_core.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <memory>

static volatile bool g_running = true;

static void sig_handler(int sig) {
    (void)sig;
    g_running = false;
}

// Subsystem adapter for Hardware commands (replaces hardware-manager container)
class HardwareSubsystemHandler : public cc::ISubsystemHandler {
public:
    explicit HardwareSubsystemHandler(std::shared_ptr<cc::HardwareRegistry> reg)
        : reg_(reg) {}

    std::string subsystem_name() const override { return "hardware"; }

    bool handle_message(const std::string& type, const nlohmann::json& payload, nlohmann::json& response) override {
        (void)type;
        response = reg_->process_json_command(payload);
        return true;
    }
private:
    std::shared_ptr<cc::HardwareRegistry> reg_;
};

// Subsystem adapter for Camera telemetry from container
class CameraSubsystemHandler : public cc::ISubsystemHandler {
public:
    explicit CameraSubsystemHandler(std::shared_ptr<cc::MavlinkCore> mavlink)
        : mavlink_(mavlink) {}

    std::string subsystem_name() const override { return "camera"; }

    bool handle_message(const std::string& type, const nlohmann::json& payload, nlohmann::json& response) override {
        if (type == "TELEMETRY") {
            auto d = payload.value("data", nlohmann::json::object());
            uint16_t w = d.value("width", 1920);
            uint16_t h = d.value("height", 1080);
            uint8_t fps = d.value("fps", 30);
            uint32_t bitrate = d.value("bitrate_kbps", 2000);
            std::string codec = d.value("codec", "H.264");
            uint8_t status = d.value("status", 2);
            mavlink_->update_camera_telemetry(w, h, fps, bitrate, codec, status);
            response = {{"status", "OK"}};
            return true;
        }
        return false;
    }
private:
    std::shared_ptr<cc::MavlinkCore> mavlink_;
};

// Subsystem adapter for Vision telemetry from container
class VisionSubsystemHandler : public cc::ISubsystemHandler {
public:
    explicit VisionSubsystemHandler(std::shared_ptr<cc::MavlinkCore> mavlink)
        : mavlink_(mavlink) {}

    std::string subsystem_name() const override { return "vision"; }

    bool handle_message(const std::string& type, const nlohmann::json& payload, nlohmann::json& response) override {
        if (type == "TELEMETRY") {
            auto d = payload.value("data", nlohmann::json::object());
            std::string model = d.value("model", "YOLOv8n");
            float conf = d.value("confidence", 0.5f);
            std::string source = d.value("input_source", "RGB");
            uint8_t status = d.value("status", 2);
            mavlink_->update_vision_telemetry(model, conf, source, status);
            response = {{"status", "OK"}};
            return true;
        }
        return false;
    }
private:
    std::shared_ptr<cc::MavlinkCore> mavlink_;
};

int main(int argc, char* argv[]) {
    std::signal(SIGINT, sig_handler);
    std::signal(SIGTERM, sig_handler);

    // 1. Dynamic Path Resolution (Zero hardcoding)
    cc::AppPaths paths = cc::AppPaths::resolve(argc, argv);

    std::string router_ip = "127.0.0.1";
    int router_port = 14600;
    if (argc >= 2 && argv[1][0] != '-') router_ip = argv[1];
    if (argc >= 3 && argv[2][0] != '-') router_port = std::stoi(argv[2]);

    std::cout << "====================================================\n";
    std::cout << "  Drone Companion Agent (Central Companion Hub)     \n";
    std::cout << "====================================================\n";
    std::cout << "[Paths] Root: " << paths.root_dir << "\n";
    std::cout << "[Paths] Run:  " << paths.run_dir << "\n";
    std::cout << "[Paths] UDS:  " << paths.cc_sock << "\n";

    // 2. Hardware Registry (Subsumes hardware-manager)
    auto hw_reg = std::make_shared<cc::HardwareRegistry>();
    std::cout << "[Hardware] Available Serial Devices:\n";
    auto ports = hw_reg->scan_devices();
    for (const auto& p : ports) {
        std::cout << "  - " << p.device_path << " (" << p.description << ")\n";
    }

    // 3. Subsystem Initializations
    cc::ConfigEngine config_engine(paths);
    cc::RollbackGuard rollback_guard(config_engine);
    cc::SystemMetricsCollector metrics;
    cc::NetworkMonitor net_monitor;

    // 4. Central UDS Hub Server (with legacy symlink to hw_manager.sock)
    auto uds_hub = std::make_shared<cc::UdsHubServer>(paths.cc_sock, paths.hw_sock);
    uds_hub->register_handler(std::make_shared<HardwareSubsystemHandler>(hw_reg));

    // 5. MAVLink Core Gateway
    auto mavlink = std::make_shared<cc::MavlinkCore>(
        router_ip, router_port, rollback_guard, metrics, net_monitor, config_engine, hw_reg, uds_hub);

    uds_hub->register_handler(std::make_shared<CameraSubsystemHandler>(mavlink));
    uds_hub->register_handler(std::make_shared<VisionSubsystemHandler>(mavlink));

    if (!uds_hub->start()) {
        std::cerr << "[Warning] Could not start UDS Hub Server!\n";
    }

    config_engine.set_log_callback([mavlink](const std::string& msg, uint8_t sev) {
        mavlink->send_statustext(msg, sev);
    });

    if (!mavlink->init()) {
        std::cerr << "[Fatal] Could not initialize MavlinkCore socket!\n";
        return 1;
    }

    mavlink->send_statustext("FC: Native C++ Companion Agent Online (CompID 191)", 6);

    auto last_1hz = std::chrono::steady_clock::now();
    bool last_fc_online = false;
    bool first_loop = true;

    while (g_running) {
        auto now = std::chrono::steady_clock::now();

        mavlink->process_incoming();

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_1hz).count() >= 1000) {
            last_1hz = now;
            rollback_guard.tick_1hz();
            config_engine.tick_1hz();
            mavlink->send_1hz_telemetry();

            // Event-driven status notification: only log on state transitions
            bool current_fc_online = rollback_guard.is_fc_online();
            if (first_loop || (current_fc_online != last_fc_online)) {
                if (current_fc_online) {
                    mavlink->send_statustext("FC Link: ONLINE (Hardware UART Active)", 6);
                } else if (!first_loop) {
                    mavlink->send_statustext("FC Link: OFFLINE (UART Inactive)", 4);
                }
                last_fc_online = current_fc_online;
                first_loop = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::cout << "[Shutdown] Native Companion Agent exiting cleanly.\n";
    uds_hub->stop();
    return 0;
}
