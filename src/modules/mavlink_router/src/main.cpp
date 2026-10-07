#include "mavlink_router/mavlink_router.hpp"
#include "mavlink_router/mavlink_router_node.hpp"
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>

static std::shared_ptr<cc::MavlinkRouter> g_router;

static void signal_handler(int sig)
{
	(void)sig;

	if (g_router) {
		g_router->stop();
	}

	if (rclcpp::ok()) {
		rclcpp::shutdown();
	}
}

int main(int argc, char **argv)
{
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	g_router = std::make_shared<cc::MavlinkRouter>();

	// Start routerd BEFORE rclcpp::init() per spec §5.9
	if (!g_router->start()) {
		std::cerr << "[mavlink_router] Failed to start routerd on boot\n";
	}

	try {
		rclcpp::init(argc, argv);
		auto hw = std::make_shared<cc::HardwareRegistry>();
		auto node = std::make_shared<cc::MavlinkRouterNode>(g_router, hw);
		rclcpp::spin(node);
		rclcpp::shutdown();

	} catch (const std::exception &e) {
		std::cerr << "[mavlink_router] ROS error: " << e.what() << ", router continues running\n";

		while (g_router->is_running()) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	if (g_router) {
		g_router->stop();
	}

	return 0;
}
