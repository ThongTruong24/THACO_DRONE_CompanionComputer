#include <gtest/gtest.h>
#include "hardware_registry.hpp"
#include "uds_hub_server.hpp"
#include "router_supervisor.hpp"
#include "nlohmann/json.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;
using json = nlohmann::json;

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

class IpcHardwareRouterIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        hw_sock = "/tmp/test_integ_hw_" + std::to_string(getpid()) + ".sock";
        router_sock = "/tmp/test_integ_router_" + std::to_string(getpid()) + ".sock";
        router_conf = "/tmp/test_integ_router_" + std::to_string(getpid()) + ".conf";

        unlink(hw_sock.c_str());
        unlink(router_sock.c_str());
        unlink(router_conf.c_str());

        // 1. Start UdsHubServer with HardwareSubsystemHandler (subsumes hardware-manager)
        hw_reg = std::make_shared<cc::HardwareRegistry>();
        hub_server = std::make_unique<cc::UdsHubServer>(hw_sock);
        hub_server->register_handler(std::make_shared<HardwareSubsystemHandler>(hw_reg));
        ASSERT_TRUE(hub_server->start());

        // 2. Start RouterSupervisor pointing to Hardware UDS
        router = std::make_unique<drone::router::RouterSupervisor>(router_sock, hw_sock, router_conf);
        ASSERT_TRUE(router->start());

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    void TearDown() override {
        if (router) router->stop();
        if (hub_server) hub_server->stop();

        unlink(hw_sock.c_str());
        unlink(router_sock.c_str());
        unlink(router_conf.c_str());
    }

    std::string send_uds(const std::string& path, const std::string& payload) {
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) return "";

        struct sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

        if (connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
            close(fd);
            return "";
        }

        std::string msg = payload + "\n";
        send(fd, msg.data(), msg.size(), 0);

        char buf[4096];
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        close(fd);

        if (n <= 0) return "";
        buf[n] = '\0';
        return std::string(buf);
    }

    std::string hw_sock;
    std::string router_sock;
    std::string router_conf;

    std::shared_ptr<cc::HardwareRegistry> hw_reg;
    std::unique_ptr<cc::UdsHubServer> hub_server;
    std::unique_ptr<drone::router::RouterSupervisor> router;
};

TEST_F(IpcHardwareRouterIntegrationTest, HardwareRegistryHealthOverSocket) {
    std::string resp = send_uds(hw_sock, R"({"cmd": "HEALTH", "id": "test-1"})");
    ASSERT_FALSE(resp.empty());
    
    json j = json::parse(resp);
    EXPECT_EQ(j["status"], "HEALTHY");
    EXPECT_TRUE(j.contains("service"));
}

TEST_F(IpcHardwareRouterIntegrationTest, RouterSupervisorHealthOverSocket) {
    std::string resp = send_uds(router_sock, R"({"cmd": "HEALTH", "id": "test-2"})");
    ASSERT_FALSE(resp.empty());

    json j = json::parse(resp);
    EXPECT_EQ(j["status"], "OK");
    EXPECT_TRUE(j.contains("router_running"));
}

TEST_F(IpcHardwareRouterIntegrationTest, EndToEndPortValidationViaRouter) {
    // When router is requested to apply config with /dev/null, it validates through hw_reg over UDS
    std::string req = R"({"cmd":"APPLY_CONFIG","id":"integ-3","fc_port":"/dev/null","fc_baud":921600})";

    std::string resp = send_uds(router_sock, req);
    ASSERT_FALSE(resp.empty());

    json j = json::parse(resp);
    EXPECT_EQ(j["status"], "OK");

    // Check that config file was written
    EXPECT_TRUE(fs::exists(router_conf));
}
