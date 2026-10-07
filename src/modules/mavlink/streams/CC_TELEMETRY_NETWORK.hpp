#pragma once

#include "../mavlink_stream.h"
#include "../mavlink_main.h"
#include "../mavlink_bridge_header.h"
#include <cc_msgs/msg/network_status.hpp>
#include <hrt/hrt.hpp>
#include <cstring>

class MavlinkStreamCcTelemetryNetwork : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamCcTelemetryNetwork(mavlink); }
	static constexpr const char *get_name_static() { return "CC_TELEMETRY_NETWORK"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK; }

	explicit MavlinkStreamCcTelemetryNetwork(Mavlink *mavlink)
		: MavlinkStream(mavlink),
		  _sub(mavlink->create_polling_subscription<cc_msgs::msg::NetworkStatus>("/cc/network_status"))
	{
		set_interval(1000000); // 1 Hz
	}

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }
	unsigned get_size() override { return MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES; }

	bool send() override
	{
		rclcpp::MessageInfo info;

		if (_sub) {
			_sub->take(_status, info);
		}

		if (_status.timestamp == 0 || (hrt_absolute_time() - _status.timestamp > Mavlink::STALE_US)) {
			return false;
		}

		mavlink_cc_telemetry_network_t msg{};
		msg.uap0_rx_kb = _status.uap0_rx_kb;
		msg.uap0_tx_kb = _status.uap0_tx_kb;
		msg.wlan0_rx_kb = _status.wlan0_rx_kb;
		msg.wlan0_tx_kb = _status.wlan0_tx_kb;
		msg.eth0_rx_kb = _status.eth0_rx_kb;
		msg.eth0_tx_kb = _status.eth0_tx_kb;
		msg.ap_channel = _status.ap_channel;
		msg.ap_ieee80211n = _status.ap_ieee80211n;
		msg.ap_wmm_enabled = _status.ap_wmm_enabled;
		msg.ap_wpa = _status.ap_wpa;
		msg.ap_client_count = _status.ap_client_count;
		msg.ap_status = _status.ap_status;
		msg.wlan0_status = _status.wlan0_status;
		msg.eth0_status = _status.eth0_status;
		msg.eth0_is_static = _status.eth0_is_static;
		msg.wlan0_dhcp = _status.wlan0_dhcp;
		msg.dnsmasq_status = _status.dnsmasq_status;
		msg.wlan0_rssi = _status.wlan0_rssi;

		std::strncpy(msg.eth0_ip, _status.eth0_ip.c_str(), sizeof(msg.eth0_ip) - 1);
		std::strncpy(msg.eth0_netmask, _status.eth0_netmask.c_str(), sizeof(msg.eth0_netmask) - 1);
		std::strncpy(msg.wlan0_ip, _status.wlan0_ip.c_str(), sizeof(msg.wlan0_ip) - 1);
		std::strncpy(msg.wlan0_netmask, _status.wlan0_netmask.c_str(), sizeof(msg.wlan0_netmask) - 1);
		std::strncpy(msg.wlan0_ssid, _status.wlan0_ssid.c_str(), sizeof(msg.wlan0_ssid) - 1);
		std::strncpy(msg.ap_ip, _status.ap_ip.c_str(), sizeof(msg.ap_ip) - 1);
		std::strncpy(msg.ap_netmask, _status.ap_netmask.c_str(), sizeof(msg.ap_netmask) - 1);
		std::strncpy(msg.ap_ssid, _status.ap_ssid.c_str(), sizeof(msg.ap_ssid) - 1);
		msg.ap_wpa_passphrase[0] = '\0'; // Passphrase never goes over topic (S4)
		std::strncpy(msg.ap_key_mgmt, _status.ap_key_mgmt.c_str(), sizeof(msg.ap_key_mgmt) - 1);
		std::strncpy(msg.ap_hw_mode, _status.ap_hw_mode.c_str(), sizeof(msg.ap_hw_mode) - 1);

		mavlink_msg_cc_telemetry_network_send_struct(_mavlink->get_channel(), &msg);
		return true;
	}

private:
	rclcpp::Subscription<cc_msgs::msg::NetworkStatus>::SharedPtr _sub;
	cc_msgs::msg::NetworkStatus _status{};
};
