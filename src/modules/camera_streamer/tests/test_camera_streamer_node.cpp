#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include "camera_streamer/camera_streamer_node.hpp"

using namespace cc;
using namespace drone;

class MockCameraDevice : public ICameraDevice
{
public:
	bool initialize(const CameraConfig &) override { init_called = true; return true; }
	bool startStream() override { is_streaming = true; start_called_count++; return true; }
	void stopStream() override { is_streaming = false; stop_called_count++; }
	bool captureFrame(FrameData &out_frame) override
	{
		out_frame.width = 640;
		out_frame.height = 480;
		out_frame.data.resize(640 * 480 * 3, 100);
		out_frame.timestamp_us = 12345;
		return true;
	}
	bool isHealthy() const override { return true; }
	bool isAvailable() const override { return true; }
	std::string getDeviceName() const override { return "MockCamera"; }
	StreamErrorCode getLastErrorCode() const override { return StreamErrorCode::SUCCESS; }

	bool init_called{false};
	bool is_streaming{false};
	int start_called_count{0};
	int stop_called_count{0};
};

class MockGstStreamSink : public IGstStreamSink
{
public:
	bool start(const CameraConfig &) override { start_called = true; return true; }
	bool pushFrame(const FrameData &) override { pushed_frames++; return true; }
	void stop() override { stop_called = true; }
	bool isConnected() const override { return true; }
	StreamErrorCode getLastErrorCode() const override { return StreamErrorCode::SUCCESS; }
	int getCurrentBitrate() const override { return current_bitrate; }
	uint64_t getDroppedFrames() const override { return 0; }
	void setBitrate(int bitrate_kbps) override { current_bitrate = bitrate_kbps; }
	void setEnabled(bool enabled) override { is_enabled = enabled; set_enabled_called_count++; }

	bool start_called{false};
	bool stop_called{false};
	bool is_enabled{true};
	int current_bitrate{1400};
	int set_enabled_called_count{0};
	uint64_t pushed_frames{0};
};

class CameraStreamerNodeTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		if (!rclcpp::ok()) {
			rclcpp::init(0, nullptr);
		}
	}

	static void TearDownTestSuite()
	{
		if (rclcpp::ok()) {
			rclcpp::shutdown();
		}
	}
};

TEST_F(CameraStreamerNodeTest, ParameterValidationBitrate)
{
	auto mock_cam = std::make_unique<MockCameraDevice>();
	auto mock_sink = std::make_unique<MockGstStreamSink>();
	auto node = std::make_shared<CameraStreamerNode>(
			    rclcpp::NodeOptions(),
			    std::move(mock_cam),
			    std::move(mock_sink)
		    );

	// Invalid bitrate: < 300
	auto res_low = node->set_parameter(rclcpp::Parameter("bitrate_kbps", 200));
	EXPECT_FALSE(res_low.successful);

	// Invalid bitrate: > 20000
	auto res_high = node->set_parameter(rclcpp::Parameter("bitrate_kbps", 25000));
	EXPECT_FALSE(res_high.successful);

	// Valid bitrate: within [300, 20000]
	auto res_ok = node->set_parameter(rclcpp::Parameter("bitrate_kbps", 3500));
	EXPECT_TRUE(res_ok.successful);
}

TEST_F(CameraStreamerNodeTest, ParameterValidationFpsAndAi)
{
	auto mock_cam = std::make_unique<MockCameraDevice>();
	auto mock_sink = std::make_unique<MockGstStreamSink>();
	auto node = std::make_shared<CameraStreamerNode>(
			    rclcpp::NodeOptions(),
			    std::move(mock_cam),
			    std::move(mock_sink)
		    );

	// Invalid fps <= 0 or > 120
	EXPECT_FALSE(node->set_parameter(rclcpp::Parameter("fps", 0)).successful);
	EXPECT_FALSE(node->set_parameter(rclcpp::Parameter("fps", 150)).successful);
	EXPECT_TRUE(node->set_parameter(rclcpp::Parameter("fps", 60)).successful);

	// Invalid ai_fps <= 0 or > 60
	EXPECT_FALSE(node->set_parameter(rclcpp::Parameter("ai_fps", 0)).successful);
	EXPECT_FALSE(node->set_parameter(rclcpp::Parameter("ai_fps", 75)).successful);
	EXPECT_TRUE(node->set_parameter(rclcpp::Parameter("ai_fps", 15)).successful);

	// Invalid ai_width / ai_height <= 0
	EXPECT_FALSE(node->set_parameter(rclcpp::Parameter("ai_width", -10)).successful);
	EXPECT_FALSE(node->set_parameter(rclcpp::Parameter("ai_height", 0)).successful);
	EXPECT_TRUE(node->set_parameter(rclcpp::Parameter("ai_width", 640)).successful);
	EXPECT_TRUE(node->set_parameter(rclcpp::Parameter("ai_height", 480)).successful);
}

TEST_F(CameraStreamerNodeTest, ToggleStateTable)
{
	auto mock_cam_ptr = std::make_unique<MockCameraDevice>();
	auto mock_sink_ptr = std::make_unique<MockGstStreamSink>();
	MockCameraDevice *raw_cam = mock_cam_ptr.get();
	MockGstStreamSink *raw_sink = mock_sink_ptr.get();

	auto node = std::make_shared<CameraStreamerNode>(
			    rclcpp::NodeOptions(),
			    std::move(mock_cam_ptr),
			    std::move(mock_sink_ptr)
		    );

	node->start();

	// Wait briefly for capture/stream thread to start
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	// State 1: Both FPV and AI enabled (default)
	EXPECT_TRUE(node->is_fpv_enabled());
	EXPECT_TRUE(node->is_ai_enabled());
	EXPECT_TRUE(raw_sink->is_enabled);
	EXPECT_TRUE(node->is_camera_streaming());

	// State 2: Disable FPV, keep AI enabled
	node->set_parameter(rclcpp::Parameter("fpv_enabled", false));
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	EXPECT_FALSE(node->is_fpv_enabled());
	EXPECT_TRUE(node->is_ai_enabled());
	EXPECT_FALSE(raw_sink->is_enabled); // Sink disabled
	EXPECT_TRUE(node->is_camera_streaming()); // Camera STILL streaming for AI!

	// State 3: Disable AI as well -> Both disabled!
	node->set_parameter(rclcpp::Parameter("ai_enabled", false));
	std::this_thread::sleep_for(std::chrono::milliseconds(150));
	EXPECT_FALSE(node->is_fpv_enabled());
	EXPECT_FALSE(node->is_ai_enabled());
	EXPECT_FALSE(raw_sink->is_enabled);
	EXPECT_FALSE(node->is_camera_streaming()); // Camera stopped via stopStream()
	EXPECT_FALSE(raw_cam->is_streaming);

	// State 4: Re-enable FPV only -> Camera restarts!
	node->set_parameter(rclcpp::Parameter("fpv_enabled", true));
	std::this_thread::sleep_for(std::chrono::milliseconds(150));
	EXPECT_TRUE(node->is_fpv_enabled());
	EXPECT_FALSE(node->is_ai_enabled());
	EXPECT_TRUE(raw_sink->is_enabled);
	EXPECT_TRUE(node->is_camera_streaming()); // Camera resumed
	EXPECT_TRUE(raw_cam->is_streaming);

	node->stop();
}

TEST_F(CameraStreamerNodeTest, StatusStringOutput)
{
	auto mock_cam = std::make_unique<MockCameraDevice>();
	auto mock_sink = std::make_unique<MockGstStreamSink>();
	auto node = std::make_shared<CameraStreamerNode>(
			    rclcpp::NodeOptions(),
			    std::move(mock_cam),
			    std::move(mock_sink)
		    );

	// Before start(): DISCONNECTED
	EXPECT_EQ(node->get_status_string(), "DISCONNECTED");

	node->start();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	EXPECT_EQ(node->get_status_string(), "STREAMING");

	// Disable both -> STANDBY
	node->set_parameter(rclcpp::Parameter("fpv_enabled", false));
	node->set_parameter(rclcpp::Parameter("ai_enabled", false));
	std::this_thread::sleep_for(std::chrono::milliseconds(150));
	EXPECT_EQ(node->get_status_string(), "STANDBY");

	node->stop();
}

TEST_F(CameraStreamerNodeTest, CameraModelParameterDeclared)
{
	auto mock_cam = std::make_unique<MockCameraDevice>();
	auto mock_sink = std::make_unique<MockGstStreamSink>();
	auto node = std::make_shared<CameraStreamerNode>(
			    rclcpp::NodeOptions(),
			    std::move(mock_cam),
			    std::move(mock_sink)
		    );

	std::string model;
	EXPECT_TRUE(node->get_parameter("camera_model", model));
	EXPECT_EQ(model, "realsense_d435i");
}
