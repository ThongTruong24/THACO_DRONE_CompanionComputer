#include "config_manager.h"

int main(int argc, char **argv)
{
	rclcpp::init(argc, argv);
	auto node = std::make_shared<cc::ConfigManager>();
	rclcpp::executors::MultiThreadedExecutor exec;
	exec.add_node(node);
	exec.spin();
	rclcpp::shutdown();
	return 0;
}
