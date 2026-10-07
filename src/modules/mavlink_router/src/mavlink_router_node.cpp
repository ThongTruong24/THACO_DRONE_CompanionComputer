#include "mavlink_router/mavlink_router_node.hpp"
#include <hrt/hrt.hpp>

namespace cc
{

MavlinkRouterNode::MavlinkRouterNode(
	std::shared_ptr<MavlinkRouter> router,
	std::shared_ptr<HardwareRegistry> hw_registry,
	const rclcpp::NodeOptions &options)
	: Node("mavlink_router", options),
	  router_(std::move(router)),
	  hw_registry_(std::move(hw_registry))
{
	apply_service_ = create_service<cc_msgs::srv::ApplyLinks>(
				 "/cc/router/apply_links",
				 std::bind(&MavlinkRouterNode::handle_apply_links, this, std::placeholders::_1, std::placeholders::_2));

	link_sub_ = create_subscription<cc_msgs::msg::LinkConfig>(
			    "/cc/link_config",
			    rclcpp::QoS(1).transient_local().reliable(),
			    std::bind(&MavlinkRouterNode::handle_link_config, this, std::placeholders::_1));

	status_pub_ = create_publisher<cc_msgs::msg::SerialLinkStatus>(
			      "/cc/serial_link_status", rclcpp::QoS(1).reliable());

	log_pub_ = create_publisher<cc_msgs::msg::MavlinkLog>(
			   "/cc/mavlink_log", rclcpp::QoS(10));

	timer_1hz_ = create_wall_timer(
			     std::chrono::seconds(1),
			     std::bind(&MavlinkRouterNode::on_timer_1hz, this));
}

void MavlinkRouterNode::handle_apply_links(
	const std::shared_ptr<cc_msgs::srv::ApplyLinks::Request> request,
	std::shared_ptr<cc_msgs::srv::ApplyLinks::Response> response)
{
	Links links;

	for (const auto &item : request->config.links) {
		links.push_back({item.name, item.port, item.baud});
	}

	std::string err;
	bool ok = router_->apply_links(links, err);
	response->success = ok;
	response->message = ok ? "Links applied" : err;
}

void MavlinkRouterNode::handle_link_config(const cc_msgs::msg::LinkConfig::SharedPtr msg)
{
	Links links;

	for (const auto &item : msg->links) {
		links.push_back({item.name, item.port, item.baud});
	}

	auto current = router_->get_current_config().links;

	if (links != current) {
		std::string err;

		if (!router_->apply_links(links, err)) {
			RCLCPP_WARN(get_logger(), "Failed to apply link config from topic: %s", err.c_str());
		}
	}
}

void MavlinkRouterNode::on_timer_1hz()
{
	auto devs = hw_registry_->scan_devices();
	std::vector<std::string> avail_ports;
	avail_ports.reserve(devs.size());

	for (const auto &d : devs) {
		avail_ports.push_back(d.device_path);
	}

	auto active_links = router_->get_current_config().links;
	auto stats = router_->get_stats();

	HwSerialStats serial_stats(hw_registry_);
	std::vector<std::string> transition_logs;

	auto status_msg = link_monitor_.tick(active_links, stats, serial_stats, avail_ports, transition_logs);
	status_pub_->publish(status_msg);

	for (const auto &log_str : transition_logs) {
		cc_msgs::msg::MavlinkLog log_msg;
		log_msg.timestamp = hrt_absolute_time();
		log_msg.text = log_str;
		log_msg.severity = 5; // NOTICE
		log_pub_->publish(log_msg);
	}
}

} // namespace cc
