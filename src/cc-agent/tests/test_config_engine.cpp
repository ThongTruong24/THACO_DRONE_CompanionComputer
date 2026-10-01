#include <gtest/gtest.h>
#include "config_engine.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace cc;

class ConfigEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / ("drone_cfg_test_" + std::to_string(getpid()));
        fs::create_directories(test_dir / "config");

        // Create dummy .env
        std::ofstream env(test_dir / ".env");
        env << "DRONE_SERIAL_PORT=/dev/ttyAMA4\n"
            << "DRONE_BAUD_RATE=921600\n"
            << "SIYI_SERIAL_PORT=/dev/ttyAMA0\n"
            << "SIYI_BAUD=115200\n";
        env.close();

        AppPaths paths;
        paths.root_dir = test_dir.string();
        paths.env_file = (test_dir / ".env").string();
        paths.config_dir = (test_dir / "config").string();
        paths.config_file = (test_dir / "config" / "active_config.yaml").string();
        paths.run_dir = (test_dir / "run").string();
        engine = std::make_unique<ConfigEngine>(paths);
    }

    void TearDown() override {
        fs::remove_all(test_dir);
    }

    fs::path test_dir;
    std::unique_ptr<ConfigEngine> engine;
};

TEST_F(ConfigEngineTest, LoadTelemetryConfigFromEnv) {
    TelemetryConfig cfg = engine->load_telemetry_config();
    EXPECT_EQ(cfg.fc_port, "/dev/ttyAMA4");
    EXPECT_EQ(cfg.fc_baud, 921600u);
    EXPECT_EQ(cfg.siyi_port, "/dev/ttyAMA0");
    EXPECT_EQ(cfg.siyi_baud, 115200u);
}

TEST_F(ConfigEngineTest, SaveTelemetryConfigUpdatesEnv) {
    TelemetryConfig new_cfg;
    new_cfg.fc_port = "/dev/ttyUSB0";
    new_cfg.fc_baud = 57600;
    new_cfg.siyi_port = "/dev/ttyUSB1";
    new_cfg.siyi_baud = 115200;

    EXPECT_TRUE(engine->save_telemetry_config(new_cfg));

    TelemetryConfig reloaded = engine->load_telemetry_config();
    EXPECT_EQ(reloaded.fc_port, "/dev/ttyUSB0");
    EXPECT_EQ(reloaded.fc_baud, 57600u);
}

TEST_F(ConfigEngineTest, TransactionCommitFlow) {
    // 1. Begin transaction
    TxResult r_begin = engine->begin(10);
    EXPECT_TRUE(r_begin.success);
    uint32_t tid = r_begin.tid;
    EXPECT_GT(tid, 0u);

    // 2. Set parameter
    TxResult r_set = engine->set(tid, "system.log_level", "DEBUG");
    EXPECT_TRUE(r_set.success);

    // 3. Staged value should be accessible via get
    auto [found, val] = engine->get("system.log_level");
    EXPECT_TRUE(found);
    EXPECT_EQ(val, "DEBUG");

    // 4. Apply transaction (transitions STAGING -> WAIT_CONFIRM)
    TxResult r_apply = engine->apply(tid);
    EXPECT_TRUE(r_apply.success);

    // 5. Confirm transaction
    TxResult r_conf = engine->confirm(tid);
    EXPECT_TRUE(r_conf.success);

    // 6. Value should persist after confirm
    auto [found2, val2] = engine->get("system.log_level");
    EXPECT_TRUE(found2);
    EXPECT_EQ(val2, "DEBUG");
}

TEST_F(ConfigEngineTest, TransactionRollbackFlow) {
    // Read initial value
    auto [init_found, init_val] = engine->get("system.log_level");

    // Begin transaction
    TxResult r_begin = engine->begin(10);
    uint32_t tid = r_begin.tid;

    // Change value
    engine->set(tid, "system.log_level", "TRACE");
    EXPECT_EQ(engine->get("system.log_level").second, "TRACE");

    // Rollback
    TxResult r_rb = engine->rollback(tid);
    EXPECT_TRUE(r_rb.success);

    // Value must revert back to initial
    auto [rb_found, rb_val] = engine->get("system.log_level");
    EXPECT_EQ(rb_val, init_val);
}

TEST_F(ConfigEngineTest, InvalidTidRejection) {
    TxResult r = engine->set(99999, "system.log_level", "ERROR");
    EXPECT_FALSE(r.success);
}
