#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "mavlink_main.h"
#include "mavlink_receiver.h"
#include "mavlink_parameters.h"
#include "mavlink_bridge_header.h"

class MavlinkReceiverTest : public ::testing::Test
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
		static int port_offset = 0;
		int port = 25600 + (port_offset++ * 2);
		mavlink_node = std::make_shared<Mavlink>("test_mavlink_rx_" + std::to_string(port),
				"127.0.0.1", port, port + 1);
		receiver = mavlink_node->get_receiver();

		status_sub = mavlink_node->create_subscription<cc_msgs::msg::VehicleStatus>(
				     "/cc/vehicle_status", 10,
		[this](const cc_msgs::msg::VehicleStatus::SharedPtr msg) {
			received_status.push_back(*msg);
		});

		cmd_sub = mavlink_node->create_subscription<cc_msgs::msg::VehicleCommand>(
				  "/cc/vehicle_command", 10,
		[this](const cc_msgs::msg::VehicleCommand::SharedPtr msg) {
			received_cmd.push_back(*msg);
		});
	}

	void TearDown() override
	{
		status_sub.reset();
		cmd_sub.reset();
		mavlink_node.reset();
	}

	void spin_cycles(int cycles = 5)
	{
		for (int i = 0; i < cycles; ++i) {
			rclcpp::spin_some(mavlink_node);
			usleep(1000);
		}
	}

	std::shared_ptr<Mavlink> mavlink_node;
	MavlinkReceiver *receiver{nullptr};

	rclcpp::Subscription<cc_msgs::msg::VehicleStatus>::SharedPtr status_sub;
	rclcpp::Subscription<cc_msgs::msg::VehicleCommand>::SharedPtr cmd_sub;

	std::vector<cc_msgs::msg::VehicleStatus> received_status;
	std::vector<cc_msgs::msg::VehicleCommand> received_cmd;
};

TEST_F(MavlinkReceiverTest, OnlyAutopilotHeartbeatPublishesArmedState)
{
	// 1. Valid autopilot HEARTBEAT with ARMED bit
	mavlink_message_t msg1;
	mavlink_heartbeat_t hb1{};
	hb1.type = MAV_TYPE_QUADROTOR;
	hb1.autopilot = MAV_AUTOPILOT_PX4;
	hb1.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;
	mavlink_msg_heartbeat_encode(1, MAV_COMP_ID_AUTOPILOT1, &msg1, &hb1);

	receiver->handle_message(&msg1);
	spin_cycles();

	ASSERT_EQ(received_status.size(), 1u);
	EXPECT_EQ(received_status[0].arming_state, cc_msgs::msg::VehicleStatus::ARMING_STATE_ARMED);

	// 2. Valid autopilot HEARTBEAT with DISARMED
	mavlink_message_t msg2;
	mavlink_heartbeat_t hb2{};
	hb2.type = MAV_TYPE_QUADROTOR;
	hb2.autopilot = MAV_AUTOPILOT_PX4;
	hb2.base_mode = 0;
	mavlink_msg_heartbeat_encode(1, MAV_COMP_ID_AUTOPILOT1, &msg2, &hb2);

	receiver->handle_message(&msg2);
	spin_cycles();

	ASSERT_EQ(received_status.size(), 2u);
	EXPECT_EQ(received_status[1].arming_state, cc_msgs::msg::VehicleStatus::ARMING_STATE_DISARMED);

	// 3. Different sysid (e.g. 2) -> ignored
	mavlink_message_t msg3;
	mavlink_msg_heartbeat_encode(2, MAV_COMP_ID_AUTOPILOT1, &msg3, &hb1);
	receiver->handle_message(&msg3);
	spin_cycles();
	EXPECT_EQ(received_status.size(), 2u);

	// 4. Compid not autopilot (e.g. 191) -> ignored
	mavlink_message_t msg4;
	mavlink_msg_heartbeat_encode(1, 191, &msg4, &hb1);
	receiver->handle_message(&msg4);
	spin_cycles();
	EXPECT_EQ(received_status.size(), 2u);

	// 5. Autopilot INVALID (0) -> ignored
	mavlink_message_t msg5;
	hb1.autopilot = MAV_AUTOPILOT_INVALID;
	mavlink_msg_heartbeat_encode(1, MAV_COMP_ID_AUTOPILOT1, &msg5, &hb1);
	receiver->handle_message(&msg5);
	spin_cycles();
	EXPECT_EQ(received_status.size(), 2u);
}

TEST_F(MavlinkReceiverTest, CommandLongTargetFiltering)
{
	// 1. Target component 191 (Onboard computer) -> published
	mavlink_message_t msg1;
	mavlink_command_long_t cmd1{};
	cmd1.command = 44011; // APPLY_CONFIG
	cmd1.target_system = 1;
	cmd1.target_component = 191;
	mavlink_msg_command_long_encode(1, 255, &msg1, &cmd1);

	receiver->handle_message(&msg1);
	spin_cycles();

	ASSERT_EQ(received_cmd.size(), 1u);
	EXPECT_EQ(received_cmd[0].command, 44011u);
	EXPECT_EQ(received_cmd[0].target_component, 191);

	// 2. Target component 0 (Broadcast) -> published
	mavlink_message_t msg2;
	mavlink_command_long_t cmd2{};
	cmd2.command = 44010; // SAVE_DEFAULT_CONFIG
	cmd2.target_system = 1;
	cmd2.target_component = 0;
	mavlink_msg_command_long_encode(1, 255, &msg2, &cmd2);

	receiver->handle_message(&msg2);
	spin_cycles();

	ASSERT_EQ(received_cmd.size(), 2u);
	EXPECT_EQ(received_cmd[1].command, 44010u);
	EXPECT_EQ(received_cmd[1].target_component, 0);

	// 3. Target component 100 (other) -> NOT published
	mavlink_message_t msg3;
	mavlink_command_long_t cmd3{};
	cmd3.command = 44012;
	cmd3.target_system = 1;
	cmd3.target_component = 100;
	mavlink_msg_command_long_encode(1, 255, &msg3, &cmd3);

	receiver->handle_message(&msg3);
	spin_cycles();

	EXPECT_EQ(received_cmd.size(), 2u); // unchanged
}
