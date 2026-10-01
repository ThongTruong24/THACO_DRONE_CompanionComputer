#include "app_paths.hpp"
#include <filesystem>
#include <cstdlib>
#include <unistd.h>

namespace fs = std::filesystem;

namespace cc {

static std::string get_exe_dir() {
    char buf[1024];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        std::error_code ec;
        auto p = fs::path(buf).parent_path();
        return p.string();
    }
    return "";
}

AppPaths AppPaths::resolve(int argc, char* argv[]) {
    AppPaths paths;

    // 1. Check CLI overrides
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--root-dir" && i + 1 < argc) {
            paths.root_dir = argv[++i];
        } else if (arg == "--run-dir" && i + 1 < argc) {
            paths.run_dir = argv[++i];
        } else if (arg == "--config-dir" && i + 1 < argc) {
            paths.config_dir = argv[++i];
        }
    }

    // 2. Check Environment Variables
    if (paths.root_dir.empty()) {
        if (const char* env_root = std::getenv("DRONE_ROOT")) {
            paths.root_dir = env_root;
        }
    }
    if (paths.run_dir.empty()) {
        if (const char* env_run = std::getenv("DRONE_RUN_DIR")) {
            paths.run_dir = env_run;
        }
    }
    if (paths.config_dir.empty()) {
        if (const char* env_cfg = std::getenv("DRONE_CONFIG_DIR")) {
            paths.config_dir = env_cfg;
        }
    }

    // 3. Fallbacks for root_dir
    if (paths.root_dir.empty()) {
        std::error_code ec;
        auto cwd = fs::current_path(ec).string();
        if (!ec && (fs::exists(cwd + "/.env") || fs::exists(cwd + "/docker-compose.yml"))) {
            paths.root_dir = cwd;
        } else {
            std::string exe_d = get_exe_dir();
            if (!exe_d.empty()) {
                // If installed in /usr/local/bin, root might be /etc/drone or /opt/drone
                if (fs::exists(exe_d + "/../../.env")) {
                    paths.root_dir = fs::canonical(exe_d + "/../..", ec).string();
                } else {
                    paths.root_dir = exe_d;
                }
            } else {
                paths.root_dir = ".";
            }
        }
    }

    // 4. Fallbacks for run_dir
    if (paths.run_dir.empty()) {
        paths.run_dir = "/run/drone";
    }

    // 5. Fallbacks for config_dir
    if (paths.config_dir.empty()) {
        if (fs::exists(paths.root_dir + "/config")) {
            paths.config_dir = fs::exists(paths.root_dir + "/deploy/config") ? (paths.root_dir + "/deploy/config") : (paths.root_dir + "/config");
        } else {
            paths.config_dir = paths.root_dir;
        }
    }

    // Ensure run_dir directory exists
    try {
        fs::create_directories(paths.run_dir);
    } catch (...) {}

    // 6. Build dependent paths
    paths.env_file    = paths.root_dir + "/.env";
    paths.config_file = paths.config_dir + "/active_config.yaml";
    paths.cc_sock     = paths.run_dir + "/cc_agent.sock";
    paths.router_sock = paths.run_dir + "/router.sock";
    paths.hw_sock     = paths.run_dir + "/hw_manager.sock";

    // System files
    paths.netplan_file   = "/etc/netplan/10-drone-netplan.yaml";
    paths.rpi_config_txt = fs::exists("/boot/firmware/config.txt") 
                           ? "/boot/firmware/config.txt" 
                           : fs::exists(paths.root_dir + "/deploy/setup/hardware/rpi5/config.txt") ? (paths.root_dir + "/deploy/setup/hardware/rpi5/config.txt") : (paths.root_dir + "/deploy/setup/hardware/rpi5/config.txt");

    // Camera config file search
    std::string candidate_cam = paths.root_dir + "/src/camera-stream-controller/camera.yaml";
    if (fs::exists(candidate_cam)) {
        paths.camera_config_file = candidate_cam;
    } else if (fs::exists(paths.config_dir + "/camera.yaml")) {
        paths.camera_config_file = paths.config_dir + "/camera.yaml";
    } else {
        paths.camera_config_file = paths.root_dir + "/camera.yaml";
    }

    return paths;
}

} // namespace cc
