#pragma once

#include <string>

namespace cc {

/**
 * @brief Centralized path resolver adhering to Single Responsibility Principle.
 * Eliminates all hardcoded user directories and provides flexible overrides via
 * CLI arguments, environment variables, or executable-relative discovery.
 */
struct AppPaths {
    std::string root_dir;           ///< Drone repository or installation root
    std::string run_dir;            ///< Runtime IPC directory (default: /run/drone)
    std::string config_dir;         ///< Persistent configuration directory
    
    std::string env_file;           ///< Path to .env file
    std::string config_file;        ///< Path to active_config.yaml
    std::string cc_sock;            ///< Path to central UDS server (/run/drone/cc_agent.sock)
    std::string router_sock;        ///< Path to mavlink-router UDS (/run/drone/router.sock)
    std::string hw_sock;            ///< Legacy hardware manager UDS (/run/drone/hw_manager.sock)
    std::string netplan_file;       ///< Netplan configuration path
    std::string rpi_config_txt;     ///< Raspberry Pi firmware config.txt
    std::string camera_config_file; ///< Camera RTSP streaming configuration YAML

    /**
     * @brief Resolve system paths dynamically.
     * Order of precedence:
     * 1. CLI arguments: --root-dir <path>, --run-dir <path>, --config-dir <path>
     * 2. Environment variables: DRONE_ROOT, DRONE_RUN_DIR, DRONE_CONFIG_DIR
     * 3. Executable-relative or standard Linux FHS fallbacks.
     */
    static AppPaths resolve(int argc = 0, char* argv[] = nullptr);
};

} // namespace cc
