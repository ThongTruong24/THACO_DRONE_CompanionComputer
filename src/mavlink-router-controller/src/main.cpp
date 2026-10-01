#include "router_supervisor.hpp"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

static volatile bool g_running = true;

static void sig_handler(int sig) {
    (void)sig;
    g_running = false;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, sig_handler);
    std::signal(SIGTERM, sig_handler);

    std::string socket_path = "/run/drone/router.sock";
    std::string hw_socket_path = "/run/drone/hw_manager.sock";

    if (argc >= 2) socket_path = argv[1];
    if (argc >= 3) hw_socket_path = argv[2];

    std::cout << "========================================================\n";
    std::cout << "  MAVLink Router Controller (Native C++20 Supervisor)   \n";
    std::cout << "========================================================\n";

    drone::router::RouterSupervisor supervisor(socket_path, hw_socket_path);
    if (!supervisor.start()) {
        std::cerr << "[Fatal] Could not start RouterSupervisor!\n";
        return 1;
    }

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "[Shutdown] Router Supervisor shutting down...\n";
    supervisor.stop();
    return 0;
}
