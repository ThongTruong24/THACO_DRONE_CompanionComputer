#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <systemlib/mavlink_log.h>
#include <cc_msgs/msg/mavlink_log.hpp>

class MavlinkLogTest : public ::testing::Test
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

TEST_F(MavlinkLogTest, PublishesAndTruncatesTo127Chars)
{
	auto node = std::make_shared<rclcpp::Node>("test_mavlink_log_node");
	auto pub = node->create_publisher<cc_msgs::msg::MavlinkLog>("/cc/mavlink_log", 10);

	cc_msgs::msg::MavlinkLog received_msg;
	bool received = false;

	auto sub = node->create_subscription<cc_msgs::msg::MavlinkLog>(
			   "/cc/mavlink_log", 10,
	[&](const cc_msgs::msg::MavlinkLog::SharedPtr msg) {
		received_msg = *msg;
		received = true;
	}
		   );

	// Create a string longer than 127 chars
	std::string long_text(150, 'A');
	mavlink_log_info(pub, "%s", long_text.c_str());

	// Spin briefly to receive the message
	auto start = std::chrono::steady_clock::now();

	while (!received && std::chrono::steady_clock::now() - start < std::chrono::seconds(1)) {
		rclcpp::spin_some(node);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	ASSERT_TRUE(received);
	EXPECT_EQ(received_msg.severity, _MSG_PRIO_INFO);
	EXPECT_EQ(received_msg.text.size(), 127u);
	EXPECT_GT(received_msg.timestamp, 0u);
}
