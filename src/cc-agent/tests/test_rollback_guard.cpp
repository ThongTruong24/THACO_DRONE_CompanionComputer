#include <gtest/gtest.h>
#include "rollback_guard.hpp"
#include <filesystem>

namespace fs = std::filesystem;
using namespace cc;

class RollbackGuardTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / ("drone_guard_test_" + std::to_string(getpid()));
        fs::create_directories(test_dir / "config");

        AppPaths paths;
        paths.root_dir = test_dir.string();
        paths.config_dir = (test_dir / "config").string();
        paths.env_file = (test_dir / ".env").string();
        paths.run_dir = (test_dir / "run").string();
        engine = std::make_unique<ConfigEngine>(paths);
        guard = std::make_unique<RollbackGuard>(*engine);
    }

    void TearDown() override {
        fs::remove_all(test_dir);
    }

    fs::path test_dir;
    std::unique_ptr<ConfigEngine> engine;
    std::unique_ptr<RollbackGuard> guard;
};

TEST_F(RollbackGuardTest, InitiallyFcOffline) {
    EXPECT_FALSE(guard->is_fc_online());
    EXPECT_EQ(guard->get_fc_status(), 0); // Disconnected / Offline
}

TEST_F(RollbackGuardTest, OnlineAfterHeartbeatReceived) {
    // Record heartbeat
    guard->record_fc_heartbeat(50);
    EXPECT_TRUE(guard->is_fc_online());
    EXPECT_GT(guard->get_fc_rx_bytes(), 0u);
}

TEST_F(RollbackGuardTest, StartTrialSwitchesState) {
    TelemetryConfig new_cfg;
    new_cfg.fc_port = "/dev/ttyUSB0";
    new_cfg.fc_baud = 115200;

    bool started = guard->start_trial(new_cfg);
    EXPECT_TRUE(started);
}

TEST_F(RollbackGuardTest, UpdateConfigInMemory) {
    TelemetryConfig new_cfg;
    new_cfg.fc_port = "/dev/ttyAMA4";
    new_cfg.fc_baud = 115200;
    new_cfg.siyi_port = "/dev/ttyAMA0";
    new_cfg.siyi_baud = 57600;

    guard->update_config(new_cfg);
    auto current = guard->get_current_config();
    EXPECT_EQ(current.fc_port, "/dev/ttyAMA4");
    EXPECT_EQ(current.fc_baud, 115200u);
    EXPECT_EQ(current.siyi_port, "/dev/ttyAMA0");
    EXPECT_EQ(current.siyi_baud, 57600u);
}
