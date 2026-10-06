#pragma once

#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <serial/hardware_registry.hpp>
#include <serial/hw_serial_stats.hpp>
#include "cc_msgs/msg/link_config.hpp"
#include "cc_msgs/msg/mavlink_log.hpp"
#include "cc_msgs/msg/serial_link_status.hpp"
#include "cc_msgs/srv/apply_links.hpp"
#include "mavlink_router/mavlink_router.hpp"
#include "mavlink_router/serial_link_monitor.hpp"

namespace cc
{

class MavlinkRouterNode : public rclcpp::Node
{
public:
	explicit MavlinkRouterNode(
		std::shared_ptr<MavlinkRouter> router,
		std::shared_ptr<HardwareRegistry> hw_registry = std::make_shared<HardwareRegistry>(),
		const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

	~MavlinkRouterNode() override = default;

private:
	void handle_apply_links(
		const std::shared_ptr<cc_msgs::srv::ApplyLinks::Request> request,
		std::shared_ptr<cc_msgs::srv::ApplyLinks::Response> response);

	void handle_link_config(const cc_msgs::msg::LinkConfig::SharedPtr msg);

	void on_timer_1hz();

	std::shared_ptr<MavlinkRouter> router_;
	std::shared_ptr<HardwareRegistry> hw_registry_;
	SerialLinkMonitor link_monitor_;

	rclcpp::Service<cc_msgs::srv::ApplyLinks>::SharedPtr apply_service_;
	rclcpp::Subscription<cc_msgs::msg::LinkConfig>::SharedPtr link_sub_;
	rclcpp::Publisher<cc_msgs::msg::SerialLinkStatus>::SharedPtr status_pub_;
	rclcpp::Publisher<cc_msgs::msg::MavlinkLog>::SharedPtr log_pub_;
	rclcpp::TimerBase::SharedPtr timer_1hz_;
};

} // namespace cc
