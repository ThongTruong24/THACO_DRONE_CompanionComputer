#include "uds_hub_server.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace cc {

UdsHubServer::UdsHubServer(const std::string& socket_path, 
                           const std::string& legacy_symlink_path)
    : socket_path_(socket_path), legacy_symlink_path_(legacy_symlink_path) {}

UdsHubServer::~UdsHubServer() {
    stop();
}

bool UdsHubServer::start() {
    if (running_.load()) return true;

    try {
        fs::path p(socket_path_);
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path());
        }
    } catch (...) {}

    ::unlink(socket_path_.c_str());

    server_fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "[UdsHubServer] Failed to create AF_UNIX socket: " << strerror(errno) << "\n";
        return false;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socket_path_.c_str(), sizeof(addr.sun_path) - 1);

    if (::bind(server_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "[UdsHubServer] Failed to bind to " << socket_path_ << ": " << strerror(errno) << "\n";
        ::close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    // Set 0666 permissions so containers sharing /run/drone can connect without permission errors
    ::chmod(socket_path_.c_str(), 0666);

    if (::listen(server_fd_, 16) < 0) {
        std::cerr << "[UdsHubServer] Failed to listen: " << strerror(errno) << "\n";
        ::close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    // Create legacy symlink if requested (e.g. hw_manager.sock -> cc_agent.sock)
    if (!legacy_symlink_path_.empty() && legacy_symlink_path_ != socket_path_) {
        std::error_code ec;
        fs::remove(legacy_symlink_path_, ec);
        fs::create_symlink(fs::path(socket_path_).filename(), legacy_symlink_path_, ec);
    }

    running_.store(true);
    server_thread_ = std::thread(&UdsHubServer::run_accept_loop, this);

    std::cout << "[UdsHubServer] Central IPC Hub listening on " << socket_path_ << "\n";
    return true;
}

void UdsHubServer::stop() {
    if (!running_.exchange(false)) return;

    if (server_fd_ >= 0) {
        ::shutdown(server_fd_, SHUT_RDWR);
        ::close(server_fd_);
        server_fd_ = -1;
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (int fd : client_fds_) {
            ::shutdown(fd, SHUT_RDWR);
            ::close(fd);
        }
        client_fds_.clear();
    }

    if (server_thread_.joinable()) {
        server_thread_.join();
    }

    ::unlink(socket_path_.c_str());
    if (!legacy_symlink_path_.empty()) {
        std::error_code ec;
        fs::remove(legacy_symlink_path_, ec);
    }
}

void UdsHubServer::register_handler(std::shared_ptr<ISubsystemHandler> handler) {
    if (!handler) return;
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    handlers_.push_back(handler);
}

void UdsHubServer::run_accept_loop() {
    while (running_.load()) {
        int client_fd = ::accept(server_fd_, nullptr, nullptr);
        if (client_fd < 0) {
            if (!running_.load()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            client_fds_.push_back(client_fd);
        }

        std::thread(&UdsHubServer::handle_client, this, client_fd).detach();
    }
}

void UdsHubServer::handle_client(int client_fd) {
    char buf[4096];
    std::string accumulated;

    while (running_.load()) {
        ssize_t n = ::recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = '\0';
        accumulated += buf;

        // Process line-delimited NDJSON frames
        size_t nl_pos;
        while ((nl_pos = accumulated.find('\n')) != std::string::npos) {
            std::string line = accumulated.substr(0, nl_pos);
            accumulated = accumulated.substr(nl_pos + 1);

            if (line.empty()) continue;

            nlohmann::json reply;
            try {
                auto req = nlohmann::json::parse(line);
                std::string target = req.value("target", "");
                std::string type   = req.value("type", req.value("cmd", ""));

                bool handled = false;
                std::vector<std::shared_ptr<ISubsystemHandler>> handlers_copy;
                {
                    std::lock_guard<std::mutex> lock(handlers_mutex_);
                    handlers_copy = handlers_;
                }

                for (const auto& h : handlers_copy) {
                    if (target.empty() || target == h->subsystem_name()) {
                        if (h->handle_message(type, req, reply)) {
                            handled = true;
                            break;
                        }
                    }
                }

                if (!handled) {
                    reply["status"] = "ERROR";
                    reply["error"]  = "Unhandled command or target: " + type;
                }

            } catch (const std::exception& e) {
                reply["status"] = "ERROR";
                reply["error"]  = std::string("JSON parsing error: ") + e.what();
            }

            std::string out = reply.dump() + "\n";
            ::send(client_fd, out.data(), out.size(), 0);
        }
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = std::find(client_fds_.begin(), client_fds_.end(), client_fd);
        if (it != client_fds_.end()) {
            client_fds_.erase(it);
        }
    }
    ::close(client_fd);
}

void UdsHubServer::broadcast(const nlohmann::json& msg) {
    std::string line = msg.dump() + "\n";
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (int fd : client_fds_) {
        ::send(fd, line.data(), line.size(), MSG_NOSIGNAL);
    }
}

void UdsHubServer::send_to_target(const std::string& target_subsystem, const nlohmann::json& msg) {
    nlohmann::json m = msg;
    m["target"] = target_subsystem;
    broadcast(m);
}

} // namespace cc
