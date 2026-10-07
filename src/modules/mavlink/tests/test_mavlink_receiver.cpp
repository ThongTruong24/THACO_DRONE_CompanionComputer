#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <array>
#include <chrono>
#include <thread>

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
		ai_control_sub.reset();
		track_point_sub.reset();
		mavlink_node.reset();
	}

	void spin_cycles(int cycles = 5)
	{
		for (int i = 0; i < cycles; ++i) {
			rclcpp::spin_some(mavlink_node);
			usleep(1000);
		}
	}

	void subscribe_ai_control()
	{
		rclcpp::QoS qos(1);
		qos.reliable().transient_local();
		ai_control_sub = mavlink_node->create_subscription<cc_msgs::msg::AiVisionControl>(
				 "/cc/ai_vision_control", qos,
		[this](const cc_msgs::msg::AiVisionControl::SharedPtr msg) {
			received_ai_control.push_back(*msg);
		});
	}

	bool wait_for_ai_control(size_t count)
	{
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
		while (received_ai_control.size() < count && std::chrono::steady_clock::now() < deadline) {
			rclcpp::spin_some(mavlink_node);
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		return received_ai_control.size() >= count;
	}

	void subscribe_track_point()
	{
		rclcpp::QoS qos(1);
		qos.reliable().durability_volatile();
		track_point_sub = mavlink_node->create_subscription<cc_msgs::msg::AiVisionTrackPoint>(
				  "/cc/ai_vision_track_point", qos,
		[this](const cc_msgs::msg::AiVisionTrackPoint::SharedPtr msg) {
			received_track_points.push_back(*msg);
		});
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
		while (track_point_sub->get_publisher_count() == 0 && std::chrono::steady_clock::now() < deadline) {
			spin_cycles();
		}
		ASSERT_EQ(track_point_sub->get_publisher_count(), 1u);
	}

	bool wait_for_track_point(size_t count)
	{
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
		while (received_track_points.size() < count && std::chrono::steady_clock::now() < deadline) {
			spin_cycles();
		}
		return received_track_points.size() >= count;
	}

	std::shared_ptr<Mavlink> mavlink_node;
	MavlinkReceiver *receiver{nullptr};

	rclcpp::Subscription<cc_msgs::msg::VehicleStatus>::SharedPtr status_sub;
	rclcpp::Subscription<cc_msgs::msg::VehicleCommand>::SharedPtr cmd_sub;
	rclcpp::Subscription<cc_msgs::msg::AiVisionControl>::SharedPtr ai_control_sub;
	rclcpp::Subscription<cc_msgs::msg::AiVisionTrackPoint>::SharedPtr track_point_sub;

	std::vector<cc_msgs::msg::VehicleStatus> received_status;
	std::vector<cc_msgs::msg::VehicleCommand> received_cmd;
	std::vector<cc_msgs::msg::AiVisionControl> received_ai_control;
	std::vector<cc_msgs::msg::AiVisionTrackPoint> received_track_points;
};

TEST_F(MavlinkReceiverTest, AiVisionControlPublishesNormalizedStates)
{
	subscribe_ai_control();
	const std::array<std::array<uint8_t, 3>, 4> controls{{
		{{1, 0, 1}}, {{1, 1, 0}}, {{2, 128, 255}}, {{1, 0, 0}}
	}};
	for (const auto &control : controls) {
		mavlink_message_t msg{};
		mavlink_msg_cc_ai_vision_control_pack(255, MAV_COMP_ID_MISSIONPLANNER, &msg,
						    control[0], control[1], control[2]);
		const auto tx_before = mavlink_node->get_total_tx_bytes();
		receiver->handle_message(&msg);
		// The receiver must only publish state; it must not send COMMAND_ACK.
		EXPECT_EQ(mavlink_node->get_total_tx_bytes(), tx_before);
		ASSERT_TRUE(wait_for_ai_control(received_ai_control.size() + 1));
		const auto &state = received_ai_control.back();
		EXPECT_GT(state.timestamp, 0u);
		EXPECT_EQ(state.bounding_box, control[0] != 0);
		EXPECT_EQ(state.tracking, control[1] != 0);
		EXPECT_EQ(state.following, control[2] != 0);
	}
	EXPECT_TRUE(received_cmd.empty());
}

TEST_F(MavlinkReceiverTest, AiVisionControlDisablingBoundingBoxClearsTrackingAndFollowing)
{
	subscribe_ai_control();
	mavlink_message_t msg{};
	mavlink_msg_cc_ai_vision_control_pack(255, MAV_COMP_ID_MISSIONPLANNER, &msg, 1, 1, 1);
	receiver->handle_message(&msg);
	ASSERT_TRUE(wait_for_ai_control(1));
	ASSERT_TRUE(received_ai_control.back().tracking);
	ASSERT_TRUE(received_ai_control.back().following);

	mavlink_msg_cc_ai_vision_control_pack(255, MAV_COMP_ID_MISSIONPLANNER, &msg, 0, 1, 1);
	receiver->handle_message(&msg);
	ASSERT_TRUE(wait_for_ai_control(2));
	EXPECT_FALSE(received_ai_control.back().bounding_box);
	EXPECT_FALSE(received_ai_control.back().tracking);
	EXPECT_FALSE(received_ai_control.back().following);
}

TEST_F(MavlinkReceiverTest, AiVisionControlLateSubscriberReceivesLatestState)
{
	mavlink_message_t msg{};
	mavlink_msg_cc_ai_vision_control_pack(255, MAV_COMP_ID_MISSIONPLANNER, &msg, 1, 1, 1);
	receiver->handle_message(&msg);
	mavlink_msg_cc_ai_vision_control_pack(255, MAV_COMP_ID_MISSIONPLANNER, &msg, 1, 0, 1);
	receiver->handle_message(&msg);

	subscribe_ai_control();
	ASSERT_TRUE(wait_for_ai_control(1));
	spin_cycles();
	ASSERT_EQ(received_ai_control.size(), 1u);
	EXPECT_TRUE(received_ai_control.back().bounding_box);
	EXPECT_FALSE(received_ai_control.back().tracking);
	EXPECT_TRUE(received_ai_control.back().following);

	const auto qos = receiver->get_ai_vision_control_pub()->get_actual_qos();
	EXPECT_EQ(qos.reliability(), rclcpp::ReliabilityPolicy::Reliable);
	EXPECT_EQ(qos.durability(), rclcpp::DurabilityPolicy::TransientLocal);
	EXPECT_EQ(qos.history(), rclcpp::HistoryPolicy::KeepLast);
	EXPECT_EQ(qos.depth(), 1u);
}

TEST_F(MavlinkReceiverTest, AiVisionControlUdpTransportReachesRosTopic)
{
	subscribe_ai_control();
	const int socket_fd = mavlink_node->get_socket_fd();
	ASSERT_GE(socket_fd, 0);
	struct sockaddr_in address {};
	socklen_t address_size = sizeof(address);
	ASSERT_EQ(getsockname(socket_fd, reinterpret_cast<struct sockaddr *>(&address), &address_size), 0);

	mavlink_message_t msg{};
	mavlink_msg_cc_ai_vision_control_pack(255, MAV_COMP_ID_MISSIONPLANNER, &msg, 1, 1, 0);
	uint8_t bytes[MAVLINK_MAX_PACKET_LEN];
	const auto length = mavlink_msg_to_send_buffer(bytes, &msg);
	ASSERT_EQ(sendto(socket_fd, bytes, length, 0,
			 reinterpret_cast<const struct sockaddr *>(&address), address_size), length);

	ASSERT_TRUE(wait_for_ai_control(1));
	EXPECT_TRUE(received_ai_control.back().bounding_box);
	EXPECT_TRUE(received_ai_control.back().tracking);
	EXPECT_FALSE(received_ai_control.back().following);
}

TEST_F(MavlinkReceiverTest, CameraTrackPointPublishesSemanticEventWithoutCommandOrAck)
{
	subscribe_track_point();
	for (const auto component : {uint8_t{191}, uint8_t{0}}) {
		mavlink_message_t msg{};
		mavlink_command_long_t command{};
		command.command = MAV_CMD_CAMERA_TRACK_POINT;
		ASSERT_EQ(command.command, 2004);
		command.target_system = 1;
		command.target_component = component;
		command.param1 = 0.25f;
		command.param2 = 0.75f;
		command.param3 = 0.15f;
		mavlink_msg_command_long_encode(255, MAV_COMP_ID_MISSIONPLANNER, &msg, &command);
		const auto count = received_track_points.size();
		const auto tx_before = mavlink_node->get_total_tx_bytes();
		receiver->handle_message(&msg);
		EXPECT_EQ(mavlink_node->get_total_tx_bytes(), tx_before);
		ASSERT_TRUE(wait_for_track_point(count + 1));
		EXPECT_GT(received_track_points.back().timestamp, 0u);
		EXPECT_FLOAT_EQ(received_track_points.back().x, command.param1);
		EXPECT_FLOAT_EQ(received_track_points.back().y, command.param2);
		EXPECT_FLOAT_EQ(received_track_points.back().radius, command.param3);
		// No generic command is emitted for ConfigManager to acknowledge later.
		EXPECT_TRUE(received_cmd.empty());
	}
}

TEST_F(MavlinkReceiverTest, OtherCommandsAndCameraComponentDoNotCreateTrackPointEvents)
{
	subscribe_track_point();
	mavlink_message_t msg{};
	mavlink_command_long_t command{};
	command.command = MAV_CMD_DO_SET_MODE;
	command.target_component = 191;
	mavlink_msg_command_long_encode(255, MAV_COMP_ID_MISSIONPLANNER, &msg, &command);
	receiver->handle_message(&msg);
	spin_cycles();
	EXPECT_TRUE(received_track_points.empty());
	ASSERT_EQ(received_cmd.size(), 1u);
	EXPECT_EQ(received_cmd.back().command, MAV_CMD_DO_SET_MODE);

	command.command = MAV_CMD_CAMERA_TRACK_POINT;
	command.target_component = 105;
	mavlink_msg_command_long_encode(255, MAV_COMP_ID_MISSIONPLANNER, &msg, &command);
	receiver->handle_message(&msg);
	spin_cycles();
	EXPECT_TRUE(received_track_points.empty());
	EXPECT_EQ(received_cmd.size(), 1u);
}

TEST_F(MavlinkReceiverTest, CameraTrackPointEventIsNotReplayedToLateSubscriber)
{
	mavlink_message_t msg{};
	mavlink_command_long_t command{};
	command.command = MAV_CMD_CAMERA_TRACK_POINT;
	command.target_component = 191;
	mavlink_msg_command_long_encode(255, MAV_COMP_ID_MISSIONPLANNER, &msg, &command);
	receiver->handle_message(&msg);
	subscribe_track_point();
	spin_cycles(20);
	EXPECT_TRUE(received_track_points.empty());
}

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
