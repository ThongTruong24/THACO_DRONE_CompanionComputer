#include <gtest/gtest.h>
#include "router_supervisor.hpp"
#include "nlohmann/json.hpp"

using namespace drone::router;
using json = nlohmann::json;

class RouterSupervisorTest : public ::testing::Test {
protected:
    void SetUp() override {
        supervisor = std::make_unique<RouterSupervisor>(
            "/tmp/test_drone_router_unit.sock",
            "/tmp/test_drone_hw_dummy.sock",
            "/tmp/test_mavlink_router_unit.conf"
        );
    }

    std::unique_ptr<RouterSupervisor> supervisor;
};

TEST_F(RouterSupervisorTest, HealthAndStatusCommands) {
    // HEALTH
    std::string req1 = R"({"cmd": "HEALTH", "id": "router-1"})";
    std::string rep1 = supervisor->process_json_command(req1);
    json resp1 = json::parse(rep1);
    EXPECT_EQ(resp1["id"], "router-1");
    EXPECT_EQ(resp1["status"], "OK");
    EXPECT_TRUE(resp1.contains("router_running"));

    // GET_STATUS
    std::string req2 = R"({"cmd": "GET_STATUS", "id": "router-2"})";
    std::string rep2 = supervisor->process_json_command(req2);
    json resp2 = json::parse(rep2);
    EXPECT_EQ(resp2["id"], "router-2");
    EXPECT_EQ(resp2["status"], "OK");
    EXPECT_TRUE(resp2.contains("fc_baud"));
}

TEST_F(RouterSupervisorTest, ApplyConfigCommand) {
    // Apply new baud rates
    std::string req = R"({
        "cmd": "APPLY_CONFIG",
        "id": "router-3",
        "fc_baud": 115200,
        "siyi_baud": 57600
    })";
    std::string rep = supervisor->process_json_command(req);
    json resp = json::parse(rep);
    EXPECT_EQ(resp["id"], "router-3");
    EXPECT_EQ(resp["status"], "OK");

    RouterConfig cfg = supervisor->get_current_config();
    EXPECT_EQ(cfg.fc_baud, 115200u);
    EXPECT_EQ(cfg.siyi_baud, 57600u);
}

TEST_F(RouterSupervisorTest, InvalidAndMalformedCommands) {
    std::string rep_unk = supervisor->process_json_command(R"({"cmd": "UNKNOWN"})");
    json resp_unk = json::parse(rep_unk);
    EXPECT_EQ(resp_unk["status"], "ERROR");

    std::string rep_bad = supervisor->process_json_command("not valid json");
    json resp_bad = json::parse(rep_bad);
    EXPECT_EQ(resp_bad["status"], "ERROR");
}
