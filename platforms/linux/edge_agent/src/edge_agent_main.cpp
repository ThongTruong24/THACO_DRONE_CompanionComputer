#include <rclcpp/rclcpp.hpp>
#include <config_manager.h>
#include <mavlink_main.h>

int main(int argc, char **argv)
{
	rclcpp::init(argc, argv);

	rclcpp::executors::MultiThreadedExecutor executor{rclcpp::ExecutorOptions(), 4};
	const rclcpp::NodeOptions options;

	auto config_manager = std::make_shared<cc::ConfigManager>(options);
	auto mavlink = std::make_shared<Mavlink>(options);

	executor.add_node(config_manager);
	executor.add_node(mavlink);

	executor.spin();

	rclcpp::shutdown();
	return 0;
}
