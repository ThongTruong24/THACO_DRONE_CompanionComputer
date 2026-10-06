#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <rclcpp/rclcpp.hpp>

#include <cc_msgs/msg/vehicle_command_ack.hpp>
#include "mavlink_stream.h"
#include "mavlink_receiver.h"
#include "mavlink_parameters.h"
#include "mavlink_bridge_header.h"

class Mavlink : public rclcpp::Node
{
public:
	static constexpr uint64_t STALE_US = 3000000; // 3 seconds timeout (spec §5.4, D16)

	explicit Mavlink(const rclcpp::NodeOptions &options = rclcpp::NodeOptions());
	Mavlink(const std::string &node_name,
		const std::string &router_ip,
		int router_port,
		int local_port,
		const rclcpp::NodeOptions &options = rclcpp::NodeOptions());
	~Mavlink() override;

	bool init_transport();
	void configure_streams_to_default();

	template<typename T>
	typename rclcpp::Subscription<T>::SharedPtr create_polling_subscription(const std::string &topic, const rclcpp::QoS &qos = 1)
	{
		rclcpp::SubscriptionOptions opts;
		opts.callback_group = _polling_cb_group;
		return create_subscription<T>(topic, qos, [](const typename T::SharedPtr) {}, opts);
	}

	bool send_bytes(const uint8_t *buf, int len);
	bool send_message(const mavlink_message_t *msg);

	void task_main();

	int get_socket_fd() const { return _sock_fd; }
	uint8_t get_sys_id() const { return _sys_id; }
	uint8_t get_comp_id() const { return _comp_id; }
	mavlink_channel_t get_channel() const { return _channel; }
	uint64_t get_total_tx_bytes() const { return _total_tx_bytes.load(std::memory_order_relaxed); }

	MavlinkParametersManager *get_parameters_manager() const { return _parameters_manager.get(); }
	MavlinkReceiver *get_receiver() const { return _receiver.get(); }
	const std::vector<std::unique_ptr<MavlinkStream>> &get_streams() const { return _streams; }

	rclcpp::Subscription<cc_msgs::msg::VehicleCommandAck>::SharedPtr get_command_ack_sub() const { return _cmd_ack_sub; }

private:
	std::string _router_ip{"127.0.0.1"};
	int _router_port{14600};
	int _local_port{14601};
	uint8_t _sys_id{1};
	uint8_t _comp_id{191};
	mavlink_channel_t _channel{MAVLINK_COMM_0};

	int _sock_fd{-1};
	struct sockaddr_in _router_addr {};
	std::mutex _send_mutex;
	std::atomic<uint64_t> _total_tx_bytes{0};

	rclcpp::CallbackGroup::SharedPtr _polling_cb_group;
	rclcpp::Subscription<cc_msgs::msg::VehicleCommandAck>::SharedPtr _cmd_ack_sub;
	rclcpp::TimerBase::SharedPtr _task_timer;

	std::unique_ptr<MavlinkParametersManager> _parameters_manager;
	std::unique_ptr<MavlinkReceiver> _receiver;
	std::vector<std::unique_ptr<MavlinkStream>> _streams;
};
