#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <hrt/hrt.hpp>

#include "mavlink_main.h"
#include "streams/CC_TELEMETRY_CAMERA.hpp"
#include "streams/CC_TELEMETRY_NETWORK.hpp"
#include "streams/CC_SERIAL_LINK.hpp"
#include "streams/STATUSTEXT.hpp"

class StreamsTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		if (!rclcpp::ok()) {
			rclcpp::init(0, nullptr);
		}
	}

	void SetUp() override
	{
		// Use random high ports to avoid collisions in tests
		static int port_offset = 0;
		int port = 24600 + (port_offset++ * 2);
		mavlink_node = std::make_shared<Mavlink>("test_mavlink_streams_" + std::to_string(port),
				"127.0.0.1", port, port + 1);
	}

	void TearDown() override
	{
		mavlink_node.reset();
	}

	std::shared_ptr<Mavlink> mavlink_node;
};

TEST_F(StreamsTest, CameraStreamSendsOnlyFreshData)
{
	auto pub = mavlink_node->create_publisher<cc_msgs::msg::CameraStatus>("/cc/camera_status", 10);
	MavlinkStreamCcTelemetryCamera stream(mavlink_node.get());

	// 1. No data yet: should not send
	EXPECT_FALSE(stream.send());

	// 2. Publish fresh data
	cc_msgs::msg::CameraStatus fresh_msg;
	fresh_msg.timestamp = hrt_absolute_time();
	fresh_msg.video_width = 1280;
	fresh_msg.video_height = 720;
	pub->publish(fresh_msg);

	// Spin briefly so intra-process DDS delivers the message
	rclcpp::spin_some(mavlink_node);

	// Fresh data available: should send
	EXPECT_TRUE(stream.send());

	// 3. Stale data (> STALE_US = 3s)
	cc_msgs::msg::CameraStatus stale_msg;
	stale_msg.timestamp = hrt_absolute_time() - (Mavlink::STALE_US + 1000000); // 4s old
	pub->publish(stale_msg);
	rclcpp::spin_some(mavlink_node);

	// Stale data: should not send
	EXPECT_FALSE(stream.send());
}

TEST_F(StreamsTest, NetworkStreamSendsOnlyFreshData)
{
	auto pub = mavlink_node->create_publisher<cc_msgs::msg::NetworkStatus>("/cc/network_status", 10);
	MavlinkStreamCcTelemetryNetwork stream(mavlink_node.get());

	// 1. No data yet
	EXPECT_FALSE(stream.send());

	// 2. Fresh data
	cc_msgs::msg::NetworkStatus fresh_msg;
	fresh_msg.timestamp = hrt_absolute_time();
	fresh_msg.ap_ssid = "THACO_TEST";
	pub->publish(fresh_msg);
	rclcpp::spin_some(mavlink_node);

	EXPECT_TRUE(stream.send());

	// 3. Stale data
	cc_msgs::msg::NetworkStatus stale_msg;
	stale_msg.timestamp = hrt_absolute_time() - (Mavlink::STALE_US + 500000);
	pub->publish(stale_msg);
	rclcpp::spin_some(mavlink_node);

	EXPECT_FALSE(stream.send());
}

TEST_F(StreamsTest, StatustextStreamDrainsOnlyWhenDataExists)
{
	auto pub = mavlink_node->create_publisher<cc_msgs::msg::MavlinkLog>("/cc/mavlink_log", 10);
	MavlinkStreamStatustext stream(mavlink_node.get());

	// 1. No log messages: should return false
	EXPECT_FALSE(stream.send());

	// 2. Publish log message
	cc_msgs::msg::MavlinkLog log_msg;
	log_msg.severity = 6;
	log_msg.text = "System test log message";
	pub->publish(log_msg);
	rclcpp::spin_some(mavlink_node);

	// Should send and return true
	EXPECT_TRUE(stream.send());

	// After draining, queue is empty: should return false
	EXPECT_FALSE(stream.send());
}
