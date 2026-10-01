#include <gtest/gtest.h>
#include <filesystem>
#include "config_manager.hpp"
#include "neon_processor.hpp"

using namespace drone;

TEST(CameraConfigManagerTest, LoadValidYamlConfig) {
    CameraConfig cfg;
    std::vector<std::string> candidates = {
        "src/camera-stream-controller/camera.yaml",
        "../src/camera-stream-controller/camera.yaml",
        "../../src/camera-stream-controller/camera.yaml",
        "../../../src/camera-stream-controller/camera.yaml",
        "../../../../src/camera-stream-controller/camera.yaml",
        "camera.yaml",
        "../camera.yaml"
    };
    std::string path;
    for (const auto& c : candidates) {
        if (std::filesystem::exists(c)) {
            path = c;
            break;
        }
    }

    bool ok = ConfigManager::loadConfig(path, cfg);
    EXPECT_TRUE(ok);

    // Verify parsed parameters
    EXPECT_GT(cfg.video.width, 0);
    EXPECT_GT(cfg.video.height, 0);
    EXPECT_GT(cfg.video.fps, 0);

    EXPECT_GT(cfg.encoder.bitrate_kbps, 0);
    EXPECT_FALSE(cfg.network.target_url.empty());
}

TEST(CameraConfigManagerTest, NonExistentYamlFailsGracefully) {
    CameraConfig cfg;
    bool ok = ConfigManager::loadConfig("/nonexistent/dummy_camera_config.yaml", cfg);
    EXPECT_FALSE(ok);
}

TEST(CameraFrameProcessorTest, NeonProcessorDummyFrame) {
    NeonFrameProcessor processor(0);
    FrameData frame;
    frame.width = 640;
    frame.height = 480;
    frame.format = PixelFormat::BGR8;
    frame.data.resize(640 * 480 * 3, 128); // 50% gray image

    processor.setRotation(0);
    EXPECT_EQ(processor.getRotation(), 0);

    EXPECT_NO_THROW(processor.process(frame));
    EXPECT_EQ(frame.width, 640);
    EXPECT_EQ(frame.height, 480);
}
