#include "mavlink_main.h"
#include "mavlink_messages.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <hrt/hrt.hpp>

mavlink_system_t mavlink_system = {1, 191};
static Mavlink *mavlink_instances[MAVLINK_COMM_NUM_BUFFERS] = {nullptr};

void mavlink_send_uart_bytes(mavlink_channel_t chan, const uint8_t *ch, int length)
{
	if (chan < MAVLINK_COMM_NUM_BUFFERS && mavlink_instances[chan]) {
		mavlink_instances[chan]->send_bytes(ch, length);
	}
}

Mavlink::Mavlink(const rclcpp::NodeOptions &options)
	: Mavlink("mavlink", "127.0.0.1", 14600, 14601, options)
{
}

Mavlink::Mavlink(const std::string &node_name,
		 const std::string &router_ip,
		 int router_port,
		 int local_port,
		 const rclcpp::NodeOptions &options)
	: Node(node_name, options),
	  _router_ip(router_ip),
	  _router_port(router_port),
	  _local_port(local_port)
{
	declare_parameter("router_ip", _router_ip);
	declare_parameter("router_port", _router_port);
	declare_parameter("local_port", _local_port);

	_router_ip = get_parameter("router_ip").as_string();
	_router_port = get_parameter("router_port").as_int();
	_local_port = get_parameter("local_port").as_int();

	if (_channel < MAVLINK_COMM_NUM_BUFFERS) {
		mavlink_instances[_channel] = this;
	}

	init_transport();

	// Callback group not added to executor for uORB-style polling subscriptions
	_polling_cb_group = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive, false);

	_parameters_manager = std::make_unique<MavlinkParametersManager>(this);
	_parameters_manager->init();

	_receiver = std::make_unique<MavlinkReceiver>(this, _parameters_manager.get());

	_cmd_ack_sub = create_polling_subscription<cc_msgs::msg::VehicleCommandAck>("/cc/vehicle_command_ack", 10);

	configure_streams_to_default();

	// Wall timer for 10ms poll loop (100 Hz)
	_task_timer = create_wall_timer(std::chrono::milliseconds(10), [this]() {
		task_main();
	});
}

Mavlink::~Mavlink()
{
	if (_channel < MAVLINK_COMM_NUM_BUFFERS && mavlink_instances[_channel] == this) {
		mavlink_instances[_channel] = nullptr;
	}

	if (_sock_fd >= 0) {
		close(_sock_fd);
		_sock_fd = -1;
	}
}

bool Mavlink::init_transport()
{
	std::memset(&_router_addr, 0, sizeof(_router_addr));
	_router_addr.sin_family = AF_INET;
	_router_addr.sin_port   = htons(_router_port);
	inet_pton(AF_INET, _router_ip.c_str(), &_router_addr.sin_addr);

	_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (_sock_fd < 0) {
		RCLCPP_ERROR(get_logger(), "Failed to create UDP socket: %s", strerror(errno));
		return false;
	}

	int flags = fcntl(_sock_fd, F_GETFL, 0);
	fcntl(_sock_fd, F_SETFL, flags | O_NONBLOCK);

	int reuse = 1;
	setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	struct sockaddr_in local {};
	local.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &local.sin_addr);
	local.sin_port = htons(_local_port);

	if (bind(_sock_fd, reinterpret_cast<struct sockaddr *>(&local), sizeof(local)) < 0) {
		RCLCPP_WARN(get_logger(), "bind() on port %d: %s", _local_port, strerror(errno));
		// continue even if port bound in test environment, but keep fd
	}

	return true;
}

void Mavlink::configure_streams_to_default()
{
	_streams.clear();
	std::vector<std::string> stream_names = {
		"HEARTBEAT",
		"STATUSTEXT",
		"CC_SERIAL_LINK",
		"CC_TELEMETRY_LINKS",
		"CC_TELEMETRY_CAMERA",
		"CC_TELEMETRY_NETWORK",
		"CC_TELEMETRY_VISION",
		"CC_TELEMETRY_SYSTEM"
	};

	for (const auto &name : stream_names) {
		MavlinkStream *s = create_mavlink_stream(name, this);

		if (s) {
			_streams.emplace_back(s);
		}
	}
}

bool Mavlink::send_bytes(const uint8_t *buf, int len)
{
	if (_sock_fd < 0 || len <= 0) {
		return false;
	}

	std::lock_guard<std::mutex> lock(_send_mutex);
	ssize_t sent = sendto(_sock_fd, buf, len, 0,
			      reinterpret_cast<struct sockaddr *>(&_router_addr),
			      sizeof(_router_addr));

	if (sent > 0) {
		_total_tx_bytes.fetch_add(sent, std::memory_order_relaxed);
		return true;
	}

	return false;
}

bool Mavlink::send_message(const mavlink_message_t *msg)
{
	uint8_t buf[MAVLINK_MAX_PACKET_LEN];
	uint16_t len = mavlink_msg_to_send_buffer(buf, msg);
	return send_bytes(buf, len);
}

void Mavlink::task_main()
{
	// ponytail: wall timer poll; upgrade to waitset/thread if lower latency needed
	if (_sock_fd >= 0) {
		uint8_t rx_buf[2048];
		struct sockaddr_in from_addr {};
		socklen_t from_len = sizeof(from_addr);
		ssize_t n;

		while ((n = recvfrom(_sock_fd, rx_buf, sizeof(rx_buf), 0,
				     reinterpret_cast<struct sockaddr *>(&from_addr), &from_len)) > 0) {
			mavlink_message_t msg;
			mavlink_status_t status;

			for (ssize_t i = 0; i < n; ++i) {
				if (mavlink_parse_char(_channel, rx_buf[i], &msg, &status)) {
					if (_receiver) {
						_receiver->handle_message(&msg);
					}
				}
			}
		}
	}

	uint64_t now = hrt_absolute_time();

	for (auto &stream : _streams) {
		stream->update(now);
	}

	if (_cmd_ack_sub) {
		cc_msgs::msg::VehicleCommandAck ack;
		rclcpp::MessageInfo ack_info;

		while (_cmd_ack_sub->take(ack, ack_info)) {
			mavlink_message_t ack_msg;
			mavlink_msg_command_ack_pack(
				_sys_id, _comp_id, &ack_msg,
				ack.command, ack.result, ack.result_param1, ack.result_param2,
				ack.target_system, ack.target_component);
			send_message(&ack_msg);
		}
	}
}
