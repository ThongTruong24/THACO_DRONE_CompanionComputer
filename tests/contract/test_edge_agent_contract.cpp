#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <deque>
#include <cstdlib>
#include <cstring>
#include <algorithm>

// THACO MAVLink dialect
#include "thaco/mavlink.h"

// ROS 2 for StartOrderTest stub publisher
#include <rclcpp/rclcpp.hpp>
#include <cc_msgs/msg/camera_status.hpp>
#include <hrt/hrt.hpp>

namespace {

std::string find_edge_agent_bin()
{
	if (const char *env = std::getenv("EDGE_AGENT_BIN")) {
		if (access(env, X_OK) == 0) {
			return env;
		}
	}

	std::vector<std::string> candidates = {
		"/home/lnh/THACO_Drone/Companion_Computer/install/edge_agent/bin/edge_agent",
		"/home/lnh/THACO_Drone/Companion_Computer/build/edge_agent/edge_agent",
		"install/edge_agent/bin/edge_agent",
		"build/edge_agent/edge_agent",
		"../install/edge_agent/bin/edge_agent",
		"../../install/edge_agent/bin/edge_agent"
	};

	for (const auto &c : candidates) {
		if (access(c.c_str(), X_OK) == 0) {
			return c;
		}
	}

	return "";
}

} // namespace

class EdgeAgentContractTest : public ::testing::Test {
protected:
	void SetUp() override
	{
		// Set DDS, domain ID, and unicast discovery on lo (WSL lo has no multicast)
		setenv("RMW_IMPLEMENTATION", "rmw_cyclonedds_cpp", 1);
		setenv("CYCLONEDDS_URI", "file:///home/lnh/THACO_Drone/Companion_Computer/tests/spike_ros2/ros2_api/cyclonedds_lo.xml", 1);
		setenv("ROS_DOMAIN_ID", "42", 1);

		std::string bin_path = find_edge_agent_bin();
		ASSERT_FALSE(bin_path.empty()) << "Could not locate edge_agent binary";

		// 1. Create UDP socket acting as mavlink-router
		router_fd = socket(AF_INET, SOCK_DGRAM, 0);
		ASSERT_GE(router_fd, 0);

		struct sockaddr_in r_addr {};
		r_addr.sin_family = AF_INET;
		r_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		r_addr.sin_port = htons(0); // ephemeral port

		ASSERT_EQ(bind(router_fd, reinterpret_cast<struct sockaddr *>(&r_addr), sizeof(r_addr)), 0);

		socklen_t len = sizeof(r_addr);
		ASSERT_EQ(getsockname(router_fd, reinterpret_cast<struct sockaddr *>(&r_addr), &len), 0);
		router_port = ntohs(r_addr.sin_port);

		// Timeout for recv
		struct timeval tv {};
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		setsockopt(router_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

		// 2. Spawn edge_agent
		child_pid = fork();
		ASSERT_GE(child_pid, 0);

		if (child_pid == 0) {
			// Child process
			std::string r_port_arg = "-p";
			std::string r_port_val = "mavlink:router_port:=" + std::to_string(router_port);
			std::string r_port_val_generic = "router_port:=" + std::to_string(router_port);

			execl(bin_path.c_str(), bin_path.c_str(),
			      "--ros-args",
			      "-p", r_port_val.c_str(),
			      "-p", r_port_val_generic.c_str(),
			      "-p", "local_port:=0",
			      "-p", "mavlink:local_port:=0",
			      nullptr);
			_exit(127);
		}

		// 3. Learn agent address from first packet (HEARTBEAT or telemetry)
		learn_agent_address();

		// Allow DDS endpoint discovery between internal nodes (config_manager <-> mavlink)
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

	void TearDown() override
	{
		if (child_pid > 0) {
			kill(child_pid, SIGTERM);
			int status = 0;
			for (int i = 0; i < 20; ++i) {
				pid_t res = waitpid(child_pid, &status, WNOHANG);
				if (res == child_pid) {
					child_pid = 0;
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
			if (child_pid > 0) {
				kill(child_pid, SIGKILL);
				waitpid(child_pid, &status, 0);
				child_pid = 0;
			}
		}

		if (router_fd >= 0) {
			close(router_fd);
			router_fd = -1;
		}

		_rx_queue.clear();
	}

	void learn_agent_address()
	{
		uint8_t buf[2048];
		struct sockaddr_in from {};
		socklen_t from_len = sizeof(from);

		ssize_t n = recvfrom(router_fd, buf, sizeof(buf), 0,
				     reinterpret_cast<struct sockaddr *>(&from), &from_len);
		ASSERT_GT(n, 0) << "Timed out waiting for first packet from edge_agent";

		agent_addr = from;
		agent_addr_len = from_len;
		has_agent_addr = true;
		std::cerr << "[LEARN] router_port=" << router_port << " agent_port=" << ntohs(agent_addr.sin_port) << std::endl;

		// Parse initial packet and queue if any
		mavlink_message_t msg;
		mavlink_status_t status;
		for (ssize_t i = 0; i < n; ++i) {
			if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &msg, &status)) {
				last_recv_msg = msg;
				_rx_queue.push_back(msg);
			}
		}
	}

	bool send_mavlink(const mavlink_message_t *msg)
	{
		if (!has_agent_addr || router_fd < 0) return false;
		uint8_t buf[MAVLINK_MAX_PACKET_LEN];
		uint16_t len = mavlink_msg_to_send_buffer(buf, msg);
		ssize_t sent = sendto(router_fd, buf, len, 0,
				      reinterpret_cast<struct sockaddr *>(&agent_addr), agent_addr_len);
		std::cerr << "[SEND_MAVLINK] sent=" << sent << " to agent_port=" << ntohs(agent_addr.sin_port)
			  << " msgid=" << msg->msgid << std::endl;
		return sent == len;
	}

	bool recv_mavlink_msg(mavlink_message_t *out_msg, uint32_t target_msgid = 0, int timeout_ms = 2000)
	{
		// 1. Check queued messages first
		for (auto it = _rx_queue.begin(); it != _rx_queue.end(); ++it) {
			if (target_msgid == 0 || it->msgid == target_msgid) {
				*out_msg = *it;
				_rx_queue.erase(it);
				return true;
			}
		}

		// 2. Poll socket for new messages
		auto start = std::chrono::steady_clock::now();
		uint8_t buf[2048];
		mavlink_status_t status;

		while (true) {
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - start).count();
			if (elapsed >= timeout_ms) {
				return false;
			}

			int remaining_ms = timeout_ms - static_cast<int>(elapsed);
			struct timeval tv {};
			tv.tv_sec = remaining_ms / 1000;
			tv.tv_usec = (remaining_ms % 1000) * 1000;
			setsockopt(router_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

			struct sockaddr_in from {};
			socklen_t from_len = sizeof(from);
			ssize_t n = recvfrom(router_fd, buf, sizeof(buf), 0,
					     reinterpret_cast<struct sockaddr *>(&from), &from_len);
			if (n <= 0) {
				continue;
			}

			mavlink_message_t parsed;
			for (ssize_t i = 0; i < n; ++i) {
				if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &parsed, &status)) {
					std::cerr << "[TEST HARNESS RX] msgid=" << parsed.msgid << std::endl;
					if (target_msgid == 0 || parsed.msgid == target_msgid) {
						*out_msg = parsed;
						return true;
					} else {
						_rx_queue.push_back(parsed);
					}
				}
			}
		}
	}

	void drain_rx(int duration_ms = 100)
	{
		_rx_queue.clear();
		uint8_t buf[2048];
		struct timeval tv {};
		tv.tv_sec = 0;
		tv.tv_usec = 20000; // 20ms
		setsockopt(router_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

		auto start = std::chrono::steady_clock::now();
		while (std::chrono::duration_cast<std::chrono::milliseconds>(
			       std::chrono::steady_clock::now() - start).count() < duration_ms) {
			recvfrom(router_fd, buf, sizeof(buf), 0, nullptr, nullptr);
		}

		mavlink_status_t *s = mavlink_get_channel_status(MAVLINK_COMM_0);
		s->parse_state = MAVLINK_PARSE_STATE_IDLE;
		s->packet_idx = 0;
	}

	int router_fd{-1};
	int router_port{0};
	pid_t child_pid{0};
	struct sockaddr_in agent_addr {};
	socklen_t agent_addr_len{sizeof(agent_addr)};
	bool has_agent_addr{false};
	mavlink_message_t last_recv_msg {};
	std::deque<mavlink_message_t> _rx_queue;
};

// 1. HEARTBEAT: compid 191, ONBOARD_CONTROLLER, autopilot INVALID
TEST_F(EdgeAgentContractTest, HeartbeatIdentity)
{
	mavlink_message_t msg;
	ASSERT_TRUE(recv_mavlink_msg(&msg, MAVLINK_MSG_ID_HEARTBEAT, 3000));

	EXPECT_EQ(msg.sysid, 1);
	EXPECT_EQ(msg.compid, 191);

	mavlink_heartbeat_t hb;
	mavlink_msg_heartbeat_decode(&msg, &hb);
	EXPECT_EQ(hb.type, MAV_TYPE_ONBOARD_CONTROLLER);
	EXPECT_EQ(hb.autopilot, MAV_AUTOPILOT_INVALID);
	EXPECT_EQ(hb.system_status, MAV_STATE_ACTIVE);
}

// 2. 44010/11/12 -> IN_PROGRESS then final result
TEST_F(EdgeAgentContractTest, CommandsProgressAndResult)
{
	drain_rx(50);

	// Test 44010 (CMD_SAVE): returns IN_PROGRESS, then final result (ACCEPTED or FAILED)
	mavlink_message_t cmd_msg;
	mavlink_command_long_t cmd{};
	cmd.target_system = 1;
	cmd.target_component = 191;
	cmd.command = 44010; // CMD_SAVE
	cmd.param1 = 1;      // LINKS
	mavlink_msg_command_long_encode(1, 255, &cmd_msg, &cmd);
	ASSERT_TRUE(send_mavlink(&cmd_msg));

	mavlink_message_t ack1;
	ASSERT_TRUE(recv_mavlink_msg(&ack1, MAVLINK_MSG_ID_COMMAND_ACK, 2000));
	mavlink_command_ack_t ack1_data;
	mavlink_msg_command_ack_decode(&ack1, &ack1_data);
	EXPECT_EQ(ack1_data.command, 44010);
	EXPECT_EQ(ack1_data.result, MAV_RESULT_IN_PROGRESS);

	mavlink_message_t ack2;
	ASSERT_TRUE(recv_mavlink_msg(&ack2, MAVLINK_MSG_ID_COMMAND_ACK, 2000));
	mavlink_command_ack_t ack2_data;
	mavlink_msg_command_ack_decode(&ack2, &ack2_data);
	EXPECT_EQ(ack2_data.command, 44010);
	EXPECT_TRUE(ack2_data.result == MAV_RESULT_ACCEPTED || ack2_data.result == MAV_RESULT_FAILED);

	// Test 44011 (CMD_APPLY): returns IN_PROGRESS, then TEMPORARILY_REJECTED (router absent)
	cmd.command = 44011;
	mavlink_msg_command_long_encode(1, 255, &cmd_msg, &cmd);
	ASSERT_TRUE(send_mavlink(&cmd_msg));

	ASSERT_TRUE(recv_mavlink_msg(&ack1, MAVLINK_MSG_ID_COMMAND_ACK, 2000));
	mavlink_msg_command_ack_decode(&ack1, &ack1_data);
	EXPECT_EQ(ack1_data.command, 44011);
	EXPECT_EQ(ack1_data.result, MAV_RESULT_IN_PROGRESS);

	ASSERT_TRUE(recv_mavlink_msg(&ack2, MAVLINK_MSG_ID_COMMAND_ACK, 2000));
	mavlink_msg_command_ack_decode(&ack2, &ack2_data);
	EXPECT_EQ(ack2_data.command, 44011);
	EXPECT_EQ(ack2_data.result, MAV_RESULT_TEMPORARILY_REJECTED);

	// Test 44012 (CMD_RESTORE): returns IN_PROGRESS, then TEMPORARILY_REJECTED (router absent)
	cmd.command = 44012;
	mavlink_msg_command_long_encode(1, 255, &cmd_msg, &cmd);
	ASSERT_TRUE(send_mavlink(&cmd_msg));

	ASSERT_TRUE(recv_mavlink_msg(&ack1, MAVLINK_MSG_ID_COMMAND_ACK, 2000));
	mavlink_msg_command_ack_decode(&ack1, &ack1_data);
	EXPECT_EQ(ack1_data.command, 44012);
	EXPECT_EQ(ack1_data.result, MAV_RESULT_IN_PROGRESS);

	ASSERT_TRUE(recv_mavlink_msg(&ack2, MAVLINK_MSG_ID_COMMAND_ACK, 2000));
	mavlink_msg_command_ack_decode(&ack2, &ack2_data);
	EXPECT_EQ(ack2_data.command, 44012);
	EXPECT_EQ(ack2_data.result, MAV_RESULT_TEMPORARILY_REJECTED);
}

// 3. Unknown command to 191 -> UNSUPPORTED; Broadcast -> no ACK
TEST_F(EdgeAgentContractTest, UnknownCommandHandling)
{
	drain_rx(50);

	// Targeted to 191
	mavlink_message_t cmd_msg;
	mavlink_command_long_t cmd{};
	cmd.target_system = 1;
	cmd.target_component = 191;
	cmd.command = 65000;
	mavlink_msg_command_long_encode(1, 255, &cmd_msg, &cmd);
	ASSERT_TRUE(send_mavlink(&cmd_msg));

	mavlink_message_t ack;
	ASSERT_TRUE(recv_mavlink_msg(&ack, MAVLINK_MSG_ID_COMMAND_ACK, 2000));
	mavlink_command_ack_t ack_data;
	mavlink_msg_command_ack_decode(&ack, &ack_data);
	EXPECT_EQ(ack_data.command, 65000);
	EXPECT_EQ(ack_data.result, MAV_RESULT_UNSUPPORTED);

	// Broadcast: target_component = 0
	cmd.target_component = 0;
	mavlink_msg_command_long_encode(1, 255, &cmd_msg, &cmd);
	ASSERT_TRUE(send_mavlink(&cmd_msg));

	// Expect NO ack for 65000 within 400ms
	bool got_ack = recv_mavlink_msg(&ack, MAVLINK_MSG_ID_COMMAND_ACK, 400);
	if (got_ack) {
		mavlink_msg_command_ack_decode(&ack, &ack_data);
		EXPECT_NE(ack_data.command, 65000);
	}
}

// 4. PARAM_EXT list/read/set; set invalid/absent -> FAILED with real value
TEST_F(EdgeAgentContractTest, ParamExtListReadSet)
{
	drain_rx(50);

	// List request
	mavlink_message_t req_msg;
	mavlink_param_ext_request_list_t req{};
	req.target_system = 1;
	req.target_component = 191;
	mavlink_msg_param_ext_request_list_encode(1, 255, &req_msg, &req);
	ASSERT_TRUE(send_mavlink(&req_msg));

	mavlink_message_t val_msg;
	ASSERT_TRUE(recv_mavlink_msg(&val_msg, MAVLINK_MSG_ID_PARAM_EXT_VALUE, 2000));
	mavlink_param_ext_value_t val;
	mavlink_msg_param_ext_value_decode(&val_msg, &val);
	EXPECT_GT(val.param_count, 0);

	// Read request for CC_AP_SSID
	mavlink_param_ext_request_read_t rreq{};
	rreq.target_system = 1;
	rreq.target_component = 191;
	std::strncpy(rreq.param_id, "CC_AP_SSID", sizeof(rreq.param_id));
	rreq.param_index = -1;
	mavlink_msg_param_ext_request_read_encode(1, 255, &req_msg, &rreq);
	ASSERT_TRUE(send_mavlink(&req_msg));

	ASSERT_TRUE(recv_mavlink_msg(&val_msg, MAVLINK_MSG_ID_PARAM_EXT_VALUE, 2000));
	mavlink_msg_param_ext_value_decode(&val_msg, &val);
	EXPECT_STREQ(val.param_id, "CC_AP_SSID");
	EXPECT_STREQ(val.param_value, "THACO_DRONE");

	// Set on absent node (camera_streamer not running) -> FAILED (result=2) with real value
	mavlink_param_ext_set_t sreq{};
	sreq.target_system = 1;
	sreq.target_component = 191;
	std::strncpy(sreq.param_id, "CC_CAM_FPS", sizeof(sreq.param_id));
	sreq.param_type = MAV_PARAM_EXT_TYPE_UINT32;
	uint32_t desired = 60;
	std::memcpy(sreq.param_value, &desired, sizeof(desired));
	mavlink_msg_param_ext_set_encode(1, 255, &req_msg, &sreq);
	ASSERT_TRUE(send_mavlink(&req_msg));

	mavlink_message_t ack_msg;
	ASSERT_TRUE(recv_mavlink_msg(&ack_msg, MAVLINK_MSG_ID_PARAM_EXT_ACK, 2000));
	mavlink_param_ext_ack_t p_ack;
	mavlink_msg_param_ext_ack_decode(&ack_msg, &p_ack);
	EXPECT_STREQ(p_ack.param_id, "CC_CAM_FPS");
	EXPECT_EQ(p_ack.param_result, 2); // PARAM_ACK_FAILED
	uint32_t returned_val = 0;
	std::memcpy(&returned_val, p_ack.param_value, sizeof(returned_val));
	EXPECT_EQ(returned_val, 30u); // returns real original value
}

// 5. Telemetry streams: CC_TELEMETRY_SYSTEM present, NO CC_TELEMETRY_CAMERA when no publisher
TEST_F(EdgeAgentContractTest, TelemetryStreamsPresence)
{
	drain_rx(50);

	mavlink_message_t sys_msg;
	ASSERT_TRUE(recv_mavlink_msg(&sys_msg, MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM, 3000));
	mavlink_cc_telemetry_system_t sys;
	mavlink_msg_cc_telemetry_system_decode(&sys_msg, &sys);
	EXPECT_EQ(sys_msg.compid, 191);
	EXPECT_GT(sys.system_uptime_s, 0u);

	// Verify NO CC_TELEMETRY_CAMERA received within 1.5 seconds
	mavlink_message_t cam_msg;
	bool got_camera = recv_mavlink_msg(&cam_msg, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA, 1500);
	EXPECT_FALSE(got_camera) << "CC_TELEMETRY_CAMERA must not be sent when no publisher exists";
}

// 6. Armed Latch: FC HEARTBEAT armed -> APPLY links -> TEMPORARILY_REJECTED + STATUSTEXT "armed"
TEST_F(EdgeAgentContractTest, ArmedLatchRejection)
{
	drain_rx(50);

	// Send FC HEARTBEAT with SAFETY_ARMED
	mavlink_message_t fc_hb;
	mavlink_heartbeat_t hb{};
	hb.type = MAV_TYPE_QUADROTOR;
	hb.autopilot = MAV_AUTOPILOT_PX4;
	hb.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;
	hb.system_status = MAV_STATE_ACTIVE;
	mavlink_msg_heartbeat_encode(1, MAV_COMP_ID_AUTOPILOT1, &fc_hb, &hb);
	ASSERT_TRUE(send_mavlink(&fc_hb));

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Send APPLY links (44011, sub=1)
	mavlink_message_t cmd_msg;
	mavlink_command_long_t cmd{};
	cmd.target_system = 1;
	cmd.target_component = 191;
	cmd.command = 44011; // CMD_APPLY
	cmd.param1 = 1;      // LINKS
	mavlink_msg_command_long_encode(1, 255, &cmd_msg, &cmd);
	ASSERT_TRUE(send_mavlink(&cmd_msg));

	// Expect IN_PROGRESS, then TEMPORARILY_REJECTED, and STATUSTEXT "armed"
	mavlink_message_t ack1;
	ASSERT_TRUE(recv_mavlink_msg(&ack1, MAVLINK_MSG_ID_COMMAND_ACK, 2000));
	mavlink_command_ack_t ack1_data;
	mavlink_msg_command_ack_decode(&ack1, &ack1_data);
	EXPECT_EQ(ack1_data.command, 44011);

	mavlink_message_t ack2;
	ASSERT_TRUE(recv_mavlink_msg(&ack2, MAVLINK_MSG_ID_COMMAND_ACK, 2000));
	mavlink_command_ack_t ack2_data;
	mavlink_msg_command_ack_decode(&ack2, &ack2_data);
	EXPECT_EQ(ack2_data.command, 44011);
	EXPECT_EQ(ack2_data.result, MAV_RESULT_TEMPORARILY_REJECTED);

	mavlink_message_t st_msg;
	ASSERT_TRUE(recv_mavlink_msg(&st_msg, MAVLINK_MSG_ID_STATUSTEXT, 2000));
	mavlink_statustext_t st;
	mavlink_msg_statustext_decode(&st_msg, &st);
	std::string text(st.text);
	std::transform(text.begin(), text.end(), text.begin(), ::tolower);
	EXPECT_NE(text.find("armed"), std::string::npos);
}

// 7. Links Staging: CC_TELEMETRY_LINKS from GCS stages CC_L0/CC_L1
TEST_F(EdgeAgentContractTest, LinksStagingFromGcs)
{
	drain_rx(50);

	mavlink_message_t links_msg;
	mavlink_cc_telemetry_links_t links{};
	std::strncpy(links.fc_port, "/dev/ttyTEST_FC", sizeof(links.fc_port));
	links.fc_baudrate = 921600;
	std::strncpy(links.siyi_port, "/dev/ttyTEST_SIYI", sizeof(links.siyi_port));
	links.siyi_baudrate = 115200;
	mavlink_msg_cc_telemetry_links_encode(1, 255, &links_msg, &links);
	ASSERT_TRUE(send_mavlink(&links_msg));

	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	// Verify CC_L0_PORT readback
	mavlink_message_t req_msg;
	mavlink_param_ext_request_read_t rreq{};
	rreq.target_system = 1;
	rreq.target_component = 191;
	std::strncpy(rreq.param_id, "CC_L0_PORT", sizeof(rreq.param_id));
	rreq.param_index = -1;
	mavlink_msg_param_ext_request_read_encode(1, 255, &req_msg, &rreq);
	ASSERT_TRUE(send_mavlink(&req_msg));

	mavlink_message_t val_msg;
	ASSERT_TRUE(recv_mavlink_msg(&val_msg, MAVLINK_MSG_ID_PARAM_EXT_VALUE, 2000));
	mavlink_param_ext_value_t val;
	mavlink_msg_param_ext_value_decode(&val_msg, &val);
	EXPECT_STREQ(val.param_id, "CC_L0_PORT");
	EXPECT_STREQ(val.param_value, "/dev/ttyTEST_FC");
}

// 8. StartOrderTest: runs agent first; stub publisher CameraStatus starts then stops
// -> CC_TELEMETRY_CAMERA appears, then disappears within STALE_US (3s) + 1s
TEST_F(EdgeAgentContractTest, StartOrderTest)
{
	drain_rx(50);

	// Agent was already started in SetUp(). Initially no camera telemetry.
	mavlink_message_t cam_msg;
	EXPECT_FALSE(recv_mavlink_msg(&cam_msg, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA, 1000));

	// Start ROS 2 stub node publishing CameraStatus
	if (!rclcpp::ok()) {
		rclcpp::init(0, nullptr);
	}

	auto stub_node = std::make_shared<rclcpp::Node>("stub_camera_publisher");
	auto pub = stub_node->create_publisher<cc_msgs::msg::CameraStatus>("/cc/camera_status", 10);

	// Publish for 2.5 seconds at 10 Hz
	for (int i = 0; i < 25; ++i) {
		cc_msgs::msg::CameraStatus status;
		status.timestamp = hrt_absolute_time();
		status.camera_id = 1;
		status.video_fps = 30;
		status.camera_name = "test_cam";
		pub->publish(status);
		rclcpp::spin_some(stub_node);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Verify CC_TELEMETRY_CAMERA is now received
	ASSERT_TRUE(recv_mavlink_msg(&cam_msg, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA, 3000));
	mavlink_cc_telemetry_camera_t cam;
	mavlink_msg_cc_telemetry_camera_decode(&cam_msg, &cam);
	EXPECT_EQ(cam.camera_id, 1);
	EXPECT_EQ(cam.video_fps, 30);
	EXPECT_STREQ(cam.camera_name, "test_cam");

	// Cease publishing. Node destroyed.
	pub.reset();
	stub_node.reset();

	// Wait 4 seconds (STALE_US is 3s, plus 1s margin)
	std::this_thread::sleep_for(std::chrono::seconds(4));
	drain_rx(100);

	// Verify CC_TELEMETRY_CAMERA is NO LONGER received
	bool still_received = recv_mavlink_msg(&cam_msg, MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA, 1500);
	EXPECT_FALSE(still_received) << "CC_TELEMETRY_CAMERA must disappear when publisher goes stale";
}
