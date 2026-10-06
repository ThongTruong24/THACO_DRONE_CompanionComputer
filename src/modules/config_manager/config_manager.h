#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <cc_msgs/msg/link_config.hpp>
#include <cc_msgs/msg/mavlink_log.hpp>
#include <cc_msgs/msg/serial_link_status.hpp>
#include <cc_msgs/msg/system_status.hpp>
#include <cc_msgs/msg/vehicle_command.hpp>
#include <cc_msgs/msg/vehicle_command_ack.hpp>
#include <cc_msgs/msg/vehicle_status.hpp>
#include <cc_msgs/srv/apply_links.hpp>

#include "arming_latch.h"
#include "hotspot_store.h"
#include "link_store.h"
#include "load_mon.h"

namespace cc
{

struct ConfigManagerOptions {
	LinkStorePaths link_paths;
	HotspotStorePaths hotspot_paths;
	PortCheck port_check_fn = nullptr;
	RouterApplyFn router_apply_fn = nullptr;
	std::function<bool()> router_ready_fn = nullptr;
	std::function<void()> networking_reload_fn = nullptr;
	bool enable_load_mon = true;
};

class ConfigManager : public rclcpp::Node
{
public:
	explicit ConfigManager(const rclcpp::NodeOptions &node_options = rclcpp::NodeOptions(),
			       const ConfigManagerOptions &options = ConfigManagerOptions());

	ArmingLatch &arming_latch() { return _arming_latch; }
	LinkStore &link_store() { return _link_store; }
	HotspotStore &hotspot_store() { return _hotspot_store; }

	void set_available_ports(const std::vector<std::string> &ports)
	{
		std::lock_guard<std::mutex> l(_ports_mu);
		_available_ports = ports;
	}

private:
	void init_parameters();
	rcl_interfaces::msg::SetParametersResult on_set_parameters(const std::vector<rclcpp::Parameter> &params);

	void handle_vehicle_command(const cc_msgs::msg::VehicleCommand::SharedPtr msg);
	void handle_vehicle_status(const cc_msgs::msg::VehicleStatus::SharedPtr msg);
	void handle_serial_link_status(const cc_msgs::msg::SerialLinkStatus::SharedPtr msg);

	void publish_ack(const cc_msgs::msg::VehicleCommand &cmd, uint8_t result);
	void publish_link_config(const Links &links);
	void publish_system_status();

	bool router_service_is_ready();
	bool apply_via_router_service(const Links &links, std::string &err);

	Links get_links_from_parameters();
	void sync_parameters_from_links(const Links &links);

	ConfigManagerOptions _options;
	ArmingLatch _arming_latch;
	LinkStore _link_store;
	HotspotStore _hotspot_store;
	LoadMon _load_mon;

	std::mutex _ports_mu;
	std::vector<std::string> _available_ports;

	rclcpp::CallbackGroup::SharedPtr _cmd_callback_group;
	rclcpp::CallbackGroup::SharedPtr _client_callback_group;

	rclcpp::Subscription<cc_msgs::msg::VehicleCommand>::SharedPtr _sub_vehicle_command;
	rclcpp::Subscription<cc_msgs::msg::VehicleStatus>::SharedPtr _sub_vehicle_status;
	rclcpp::Subscription<cc_msgs::msg::SerialLinkStatus>::SharedPtr _sub_serial_link_status;

	rclcpp::Publisher<cc_msgs::msg::VehicleCommandAck>::SharedPtr _pub_vehicle_command_ack;
	rclcpp::Publisher<cc_msgs::msg::LinkConfig>::SharedPtr _pub_link_config;
	rclcpp::Publisher<cc_msgs::msg::MavlinkLog>::SharedPtr _pub_mavlink_log;
	rclcpp::Publisher<cc_msgs::msg::SystemStatus>::SharedPtr _pub_system_status;

	rclcpp::Client<cc_msgs::srv::ApplyLinks>::SharedPtr _router_client;
	rclcpp::TimerBase::SharedPtr _load_mon_timer;

	rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr _params_callback_handle;
};

} // namespace cc
