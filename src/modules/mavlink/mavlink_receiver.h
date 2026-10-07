#pragma once

#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <cc_msgs/msg/vehicle_status.hpp>
#include <cc_msgs/msg/vehicle_command.hpp>

class Mavlink;
class MavlinkParametersManager;
struct __mavlink_message;
typedef struct __mavlink_message mavlink_message_t;

class MavlinkReceiver
{
public:
	explicit MavlinkReceiver(Mavlink *mavlink, MavlinkParametersManager *parameters_manager);
	~MavlinkReceiver() = default;

	void handle_message(const mavlink_message_t *msg);

	rclcpp::Publisher<cc_msgs::msg::VehicleStatus>::SharedPtr get_vehicle_status_pub() const { return _vehicle_status_pub; }
	rclcpp::Publisher<cc_msgs::msg::VehicleCommand>::SharedPtr get_vehicle_command_pub() const { return _vehicle_command_pub; }

private:
	void handle_message_heartbeat(const mavlink_message_t *msg);
	void handle_message_command_long(const mavlink_message_t *msg);
	void handle_message_cc_telemetry_links(const mavlink_message_t *msg);
	void handle_message_cc_telemetry_camera(const mavlink_message_t *msg);
	void handle_message_cc_telemetry_network(const mavlink_message_t *msg);
	void handle_message_cc_telemetry_vision(const mavlink_message_t *msg);

	Mavlink *_mavlink{nullptr};
	MavlinkParametersManager *_parameters_manager{nullptr};

	rclcpp::Publisher<cc_msgs::msg::VehicleStatus>::SharedPtr _vehicle_status_pub;
	rclcpp::Publisher<cc_msgs::msg::VehicleCommand>::SharedPtr _vehicle_command_pub;
};
