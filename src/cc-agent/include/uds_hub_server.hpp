#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include "nlohmann/json.hpp"

namespace cc {

/**
 * @brief Interface Segregation Principle: Subsystem message handler abstraction.
 */
class ISubsystemHandler {
public:
    virtual ~ISubsystemHandler() = default;
    virtual std::string subsystem_name() const = 0;
    virtual bool handle_message(const std::string& type, 
                                const nlohmann::json& payload, 
                                nlohmann::json& response) = 0;
};

/**
 * @brief Central Unix Domain Socket Server (NDJSON Stream).
 * Adheres to Open/Closed Principle: Subsystem handlers can be registered dynamically
 * without modifying the core server accept and framing loop.
 */
class UdsHubServer {
public:
    explicit UdsHubServer(const std::string& socket_path, 
                          const std::string& legacy_symlink_path = "");
    ~UdsHubServer();

    bool start();
    void stop();

    void register_handler(std::shared_ptr<ISubsystemHandler> handler);

    // Broadcast a JSON command/event to all currently connected clients or target
    void broadcast(const nlohmann::json& msg);
    void send_to_target(const std::string& target_subsystem, const nlohmann::json& msg);

private:
    void run_accept_loop();
    void handle_client(int client_fd);

    std::string socket_path_;
    std::string legacy_symlink_path_;
    int server_fd_{-1};
    std::atomic<bool> running_{false};
    std::thread server_thread_;

    std::mutex handlers_mutex_;
    std::vector<std::shared_ptr<ISubsystemHandler>> handlers_;

    std::mutex clients_mutex_;
    std::vector<int> client_fds_;
};

} // namespace cc
