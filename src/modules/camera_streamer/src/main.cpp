#include <rclcpp/rclcpp.hpp>
#include "camera_streamer/camera_streamer_node.hpp"
#include <csignal>
#include <iostream>
#include <memory>

int main(int argc, char **argv)
{
	rclcpp::init(argc, argv);

	auto node = std::make_shared<cc::CameraStreamerNode>();
	node->start();

	RCLCPP_INFO(node->get_logger(), "CameraStreamerNode started successfully");

	try {
		rclcpp::spin(node);

	} catch (const std::exception &e) {
		RCLCPP_ERROR(node->get_logger(), "Exception in spin: %s", e.what());
	}

	node->stop();
	rclcpp::shutdown();
	return 0;
}
