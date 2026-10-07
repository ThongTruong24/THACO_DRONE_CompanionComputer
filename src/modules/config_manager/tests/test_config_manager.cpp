#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>
#include <atomic>

#include <rclcpp/rclcpp.hpp>
#include <cc_msgs/msg/vehicle_command.hpp>
#include <cc_msgs/msg/vehicle_command_ack.hpp>
#include <cc_msgs/msg/vehicle_status.hpp>
#include <cc_msgs/msg/mavlink_log.hpp>
#include <cc_msgs/msg/link_config.hpp>
#include <cc_msgs/msg/serial_link_status.hpp>
#include <cc_msgs/srv/apply_links.hpp>

#include "config_manager.h"

namespace fs = std::filesystem;
using namespace std::chrono_literals;

class ConfigManagerTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		rclcpp::init(0, nullptr);
	}

	static void TearDownTestSuite()
	{
		rclcpp::shutdown();
	}

	void SetUp() override
	{
		_test_dir = fs::temp_directory_path() / ("test_config_manager_" + std::to_string(std::rand()));
		fs::create_directories(_test_dir);

		_opts.link_paths.active_file = (_test_dir / "links_active.json").string();
		_opts.link_paths.default_file = (_test_dir / "links_default.json").string();
		_opts.hotspot_paths.active_file = (_test_dir / "hotspot_active.json").string();
		_opts.hotspot_paths.wifi_file = (_test_dir / "wifi.json").string();
		_opts.enable_load_mon = false; // Disable load mon timer during test
		_opts.port_check_fn = [](const std::string &) { return true; };

		// Create default links
		cc::Links def_links = {{"FC", "/dev/ttyAMA4", 921600}};
		ASSERT_TRUE(cc::save_links_file(_opts.link_paths.default_file, def_links));

		// Create default wifi
		nlohmann::json wifi = {
			{
				"hotspot", {
					{"ssid", "DEFAULT_WIFI"},
					{"password", "pass123456"}
				}
			}
		};
		std::ofstream out(_opts.hotspot_paths.wifi_file);
		out << wifi.dump(2);
		out.close();

		_cfg_node = std::make_shared<cc::ConfigManager>(rclcpp::NodeOptions(), _opts);

		_test_node = std::make_shared<rclcpp::Node>("test_harness_" + std::to_string(std::rand()));

		_pub_cmd = _test_node->create_publisher<cc_msgs::msg::VehicleCommand>("/cc/vehicle_command", 10);
		_pub_status = _test_node->create_publisher<cc_msgs::msg::VehicleStatus>(
				      "/cc/vehicle_status", rclcpp::QoS(1).transient_local());
		_pub_serial = _test_node->create_publisher<cc_msgs::msg::SerialLinkStatus>("/cc/serial_link_status", 10);

		_sub_ack = _test_node->create_subscription<cc_msgs::msg::VehicleCommandAck>(
				   "/cc/vehicle_command_ack", 10,
		[this](const cc_msgs::msg::VehicleCommandAck::SharedPtr msg) {
			std::lock_guard<std::mutex> l(_mu);
			_received_acks.push_back(*msg);
		});

		_sub_log = _test_node->create_subscription<cc_msgs::msg::MavlinkLog>(
				   "/cc/mavlink_log", 10,
		[this](const cc_msgs::msg::MavlinkLog::SharedPtr msg) {
			std::lock_guard<std::mutex> l(_mu);
			_received_logs.push_back(*msg);
		});

		// Create fake router service
		_router_srv = _test_node->create_service<cc_msgs::srv::ApplyLinks>(
				      "/cc/router/apply_links",
				      [this](const std::shared_ptr<cc_msgs::srv::ApplyLinks::Request> req,
		std::shared_ptr<cc_msgs::srv::ApplyLinks::Response> resp) {
			_router_call_count++;
			_last_applied_links_count = req->config.links.size();
			resp->success = _router_service_success;
			resp->message = _router_service_success ? "OK" : "Router error";
		});

		_exec = std::make_shared<rclcpp::executors::MultiThreadedExecutor>();
		_exec->add_node(_cfg_node);
		_exec->add_node(_test_node);

		_spin_thread = std::thread([this]() {
			_exec->spin();
		});
	}

	void TearDown() override
	{
		_exec->cancel();

		if (_spin_thread.joinable()) {
			_spin_thread.join();
		}

		_exec.reset();
		_cfg_node.reset();
		_test_node.reset();

		std::error_code ec;
		fs::remove_all(_test_dir, ec);
	}

	void send_command(uint32_t command, uint8_t target_comp, float param1 = 0.0f)
	{
		cc_msgs::msg::VehicleCommand cmd;
		cmd.timestamp = 1000;
		cmd.command = command;
		cmd.target_component = target_comp;
		cmd.source_system = 1;
		cmd.source_component = 1;
		cmd.param1 = param1;
		_pub_cmd->publish(cmd);
	}

	bool wait_for_acks(size_t count, std::chrono::milliseconds timeout = 2000ms)
	{
		auto start = std::chrono::steady_clock::now();

		while (std::chrono::steady_clock::now() - start < timeout) {
			{
				std::lock_guard<std::mutex> l(_mu);

				if (_received_acks.size() >= count) {
					return true;
				}
			}
			std::this_thread::sleep_for(10ms);
		}

		return false;
	}

	void clear_received()
	{
		std::lock_guard<std::mutex> l(_mu);
		_received_acks.clear();
		_received_logs.clear();
	}

	fs::path _test_dir;
	cc::ConfigManagerOptions _opts;
	std::shared_ptr<cc::ConfigManager> _cfg_node;
	std::shared_ptr<rclcpp::Node> _test_node;
	std::shared_ptr<rclcpp::executors::MultiThreadedExecutor> _exec;
	std::thread _spin_thread;

	rclcpp::Publisher<cc_msgs::msg::VehicleCommand>::SharedPtr _pub_cmd;
	rclcpp::Publisher<cc_msgs::msg::VehicleStatus>::SharedPtr _pub_status;
	rclcpp::Publisher<cc_msgs::msg::SerialLinkStatus>::SharedPtr _pub_serial;
	rclcpp::Subscription<cc_msgs::msg::VehicleCommandAck>::SharedPtr _sub_ack;
	rclcpp::Subscription<cc_msgs::msg::MavlinkLog>::SharedPtr _sub_log;
	rclcpp::Service<cc_msgs::srv::ApplyLinks>::SharedPtr _router_srv;

	std::mutex _mu;
	std::vector<cc_msgs::msg::VehicleCommandAck> _received_acks;
	std::vector<cc_msgs::msg::MavlinkLog> _received_logs;

	std::atomic<bool> _router_service_success{true};
	std::atomic<int> _router_call_count{0};
	std::atomic<size_t> _last_applied_links_count{0};
};

TEST_F(ConfigManagerTest, UnknownCommandTargetedTo191ReturnsUnsupported)
{
	clear_received();
	// Send unknown command 99999 targeting 191
	send_command(99999, 191);

	ASSERT_TRUE(wait_for_acks(1));
	EXPECT_EQ(_received_acks[0].command, 99999u);
	EXPECT_EQ(_received_acks[0].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_UNSUPPORTED);
}

TEST_F(ConfigManagerTest, BroadcastUnknownCommandIsIgnoredWithoutAck)
{
	clear_received();
	// Send unknown command 99999 broadcast (target_comp == 0)
	send_command(99999, 0);

	// Wait briefly; should not produce any ACK
	EXPECT_FALSE(wait_for_acks(1, 200ms));
	EXPECT_TRUE(_received_acks.empty());
}

TEST_F(ConfigManagerTest, ApplyLinksProducesInProgressThenAccepted)
{
	clear_received();
	// Command 44011 (APPLY), sub 1 (LINKS)
	send_command(44011, 191, 1.0f);

	ASSERT_TRUE(wait_for_acks(2));
	EXPECT_EQ(_received_acks[0].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_IN_PROGRESS);
	EXPECT_EQ(_received_acks[1].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);
	EXPECT_GT(_router_call_count, 0);
}

TEST_F(ConfigManagerTest, ArmedBlocksApplyAndRestoreLinks)
{
	// Send vehicle_status ARMED
	cc_msgs::msg::VehicleStatus st;
	st.arming_state = cc_msgs::msg::VehicleStatus::ARMING_STATE_ARMED;
	_pub_status->publish(st);

	// Wait for status to be processed
	std::this_thread::sleep_for(100ms);

	int router_before = _router_call_count;

	// Attempt APPLY LINKS (cmd 44011, sub 1)
	clear_received();
	send_command(44011, 191, 1.0f);

	ASSERT_TRUE(wait_for_acks(2));
	EXPECT_EQ(_received_acks[0].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_IN_PROGRESS);
	EXPECT_EQ(_received_acks[1].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED);

	// Verify router was NOT called
	EXPECT_EQ(_router_call_count, router_before);

	// Verify log contains armed message
	bool found_log = false;

	for (const auto &log : _received_logs) {
		if (log.text.find("armed") != std::string::npos || log.text.find("ARMED") != std::string::npos) {
			found_log = true;
			break;
		}
	}

	EXPECT_TRUE(found_log);

	// Attempt RESTORE LINKS (cmd 44012, sub 1) while armed
	clear_received();
	send_command(44012, 191, 1.0f);

	ASSERT_TRUE(wait_for_acks(2));
	EXPECT_EQ(_received_acks[0].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_IN_PROGRESS);
	EXPECT_EQ(_received_acks[1].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED);
	EXPECT_EQ(_router_call_count, router_before);
}

TEST_F(ConfigManagerTest, ArmedSub0RejectsEntireCommandAtomically)
{
	// Set vehicle status to ARMED
	cc_msgs::msg::VehicleStatus st;
	st.arming_state = cc_msgs::msg::VehicleStatus::ARMING_STATE_ARMED;
	_pub_status->publish(st);
	std::this_thread::sleep_for(100ms);

	clear_received();
	// Command 44011 (APPLY), sub 0 (ALL)
	send_command(44011, 191, 0.0f);

	ASSERT_TRUE(wait_for_acks(2));
	EXPECT_EQ(_received_acks[0].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_IN_PROGRESS);
	EXPECT_EQ(_received_acks[1].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED);

	// Verify active hotspot was NOT created
	EXPECT_FALSE(fs::exists(_opts.hotspot_paths.active_file));
}

TEST_F(ConfigManagerTest, ArmedDoesNotLockHotspotSub3)
{
	// Set vehicle status to ARMED
	cc_msgs::msg::VehicleStatus st;
	st.arming_state = cc_msgs::msg::VehicleStatus::ARMING_STATE_ARMED;
	_pub_status->publish(st);
	std::this_thread::sleep_for(100ms);

	clear_received();
	// Command 44011 (APPLY), sub 3 (HOTSPOT)
	send_command(44011, 191, 3.0f);

	ASSERT_TRUE(wait_for_acks(2));
	EXPECT_EQ(_received_acks[0].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_IN_PROGRESS);
	EXPECT_EQ(_received_acks[1].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

	// Verify hotspot active file exists
	EXPECT_TRUE(fs::exists(_opts.hotspot_paths.active_file));
}

TEST_F(ConfigManagerTest, RouterNotReadyReturnsTemporarilyRejected)
{
	// Drop the fake router service
	_router_srv.reset();

	clear_received();
	// Command 44011 (APPLY), sub 1 (LINKS)
	send_command(44011, 191, 1.0f);

	ASSERT_TRUE(wait_for_acks(2));
	EXPECT_EQ(_received_acks[0].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_IN_PROGRESS);
	EXPECT_EQ(_received_acks[1].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED);

	// Verify log contains router not ready
	bool found_log = false;

	for (const auto &log : _received_logs) {
		if (log.text.find("router not ready") != std::string::npos) {
			found_log = true;
			break;
		}
	}

	EXPECT_TRUE(found_log);
}

TEST_F(ConfigManagerTest, CameraAndVisionSubSemantics)
{
	clear_received();
	// Camera APPLY (sub 2) -> ACCEPTED
	send_command(44011, 191, 2.0f);
	ASSERT_TRUE(wait_for_acks(2));
	EXPECT_EQ(_received_acks[0].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_IN_PROGRESS);
	EXPECT_EQ(_received_acks[1].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_ACCEPTED);

	clear_received();
	// Camera SAVE (cmd 44010, sub 2) -> UNSUPPORTED
	send_command(44010, 191, 2.0f);
	ASSERT_TRUE(wait_for_acks(2));
	EXPECT_EQ(_received_acks[1].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_UNSUPPORTED);

	clear_received();
	// Vision APPLY (sub 4) -> UNSUPPORTED
	send_command(44011, 191, 4.0f);
	ASSERT_TRUE(wait_for_acks(2));
	EXPECT_EQ(_received_acks[1].result, cc_msgs::msg::VehicleCommandAck::VEHICLE_CMD_RESULT_UNSUPPORTED);
}

TEST_F(ConfigManagerTest, ParameterValidationWithAvailablePorts)
{
	// Set available_ports on config_manager
	_cfg_node->set_available_ports({"/dev/ttyAMA4", "/dev/ttyAMA0"});

	// Setting link0.port to an available port should succeed
	auto res1 = _cfg_node->set_parameter(rclcpp::Parameter("link0.port", "/dev/ttyAMA0"));
	EXPECT_TRUE(res1.successful);

	// Setting link0.port to an unavailable port should fail
	auto res2 = _cfg_node->set_parameter(rclcpp::Parameter("link0.port", "/dev/ttyUSB99"));
	EXPECT_FALSE(res2.successful);
	EXPECT_NE(res2.reason.find("available_ports"), std::string::npos);

	// Setting non-standard baud rate should fail
	auto res3 = _cfg_node->set_parameter(rclcpp::Parameter("link0.baud", static_cast<int64_t>(12345)));
	EXPECT_FALSE(res3.successful);

	// Setting valid standard baud rate should succeed
	auto res4 = _cfg_node->set_parameter(rclcpp::Parameter("link0.baud", static_cast<int64_t>(115200)));
	EXPECT_TRUE(res4.successful);

	// Setting short passphrase should fail
	auto res5 = _cfg_node->set_parameter(rclcpp::Parameter("ap.pass", "short"));
	EXPECT_FALSE(res5.successful);

	// Setting valid passphrase should succeed
	auto res6 = _cfg_node->set_parameter(rclcpp::Parameter("ap.pass", "validpassphrase"));
	EXPECT_TRUE(res6.successful);
}
