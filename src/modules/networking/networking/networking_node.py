"""
networking_node.py - ROS 2 Node for Network Status telemetry and Hotspot Reload.
Publishes /cc/network_status at 1 Hz.
Provides /cc/networking/reload service (std_srvs/srv/Trigger):
- If configuration is invalid -> returns success = False, AP stays untouched.
- If configuration is valid -> responds success = True, then exits with code 3
  allowing Docker container restart policy to reboot networking with fresh config.
"""
import os
import sys
import threading
import time
from typing import Callable, Optional

import rclpy
from rclpy.node import Node
from std_srvs.srv import Trigger

from cc_msgs.msg import NetworkStatus

from networking.hotspot_config import (
    is_valid_ssid,
    is_valid_wpa_passphrase,
    read_effective_hotspot,
    detect_frequency_channel,
)
from networking.net_status import (
    parse_net_dev,
    parse_arp,
    parse_wlan_rssi,
    get_interface_ip_and_netmask,
    get_interface_status,
    get_wlan_ssid,
)


class NetworkingNode(Node):
    """ROS 2 Node for Network telemetry and safe configuration reload."""

    def __init__(
        self,
        node_name: str = "networking_node",
        exit_fn: Optional[Callable[[int], None]] = None,
        proc_net_dev_path: str = "/proc/net/dev",
        proc_net_arp_path: str = "/proc/net/arp",
        sys_wireless_link_path: str = "/sys/class/net/wlan0/wireless/link",
    ):
        super().__init__(node_name)
        self._exit_fn = exit_fn or (lambda code: os._exit(code))
        self._proc_net_dev_path = proc_net_dev_path
        self._proc_net_arp_path = proc_net_arp_path
        self._sys_wireless_link_path = sys_wireless_link_path

        # Parameters
        self.declare_parameter("publish_rate_hz", 1.0)
        self.declare_parameter("active_hotspot_path", "/run/drone/hotspot.json")
        self.declare_parameter("default_wifi_path", "/app/config/wifi.json")
        self.declare_parameter("ap_iface", os.environ.get("AP_IFACE", "uap0"))
        self.declare_parameter("wlan_parent", os.environ.get("WIFI_PARENT", "wlan0"))

        rate = self.get_parameter("publish_rate_hz").value
        period = 1.0 / max(0.1, rate)

        # Publisher for telemetry
        self._pub_status = self.create_publisher(NetworkStatus, "/cc/network_status", 10)

        # Service for safe reload
        self._srv_reload = self.create_service(
            Trigger,
            "/cc/networking/reload",
            self._handle_reload_service,
        )

        # Timer for 1 Hz publish
        self._timer = self.create_timer(period, self._publish_status)

        self.get_logger().info(f"NetworkingNode started, publishing /cc/network_status at {rate} Hz")

    def _collect_network_status(self) -> NetworkStatus:
        msg = NetworkStatus()
        msg.timestamp = int(time.time() * 1_000_000)

        # 1. Dev counters
        try:
            if os.path.exists(self._proc_net_dev_path):
                with open(self._proc_net_dev_path, "r", encoding="ascii", errors="ignore") as f:
                    dev_stats = parse_net_dev(f.read())
                    if "uap0" in dev_stats:
                        msg.uap0_rx_kb = dev_stats["uap0"]["rx_kb"]
                        msg.uap0_tx_kb = dev_stats["uap0"]["tx_kb"]
                    if "wlan0" in dev_stats:
                        msg.wlan0_rx_kb = dev_stats["wlan0"]["rx_kb"]
                        msg.wlan0_tx_kb = dev_stats["wlan0"]["tx_kb"]
                    if "eth0" in dev_stats:
                        msg.eth0_rx_kb = dev_stats["eth0"]["rx_kb"]
                        msg.eth0_tx_kb = dev_stats["eth0"]["tx_kb"]
        except Exception as e:
            self.get_logger().debug(f"Failed to read /proc/net/dev: {e}")

        # 2. ARP client count
        try:
            if os.path.exists(self._proc_net_arp_path):
                with open(self._proc_net_arp_path, "r", encoding="ascii", errors="ignore") as f:
                    msg.ap_client_count = parse_arp(f.read(), subnet_prefix="192.168.10.")
        except Exception as e:
            self.get_logger().debug(f"Failed to read /proc/net/arp: {e}")

        # 3. WLAN RSSI
        try:
            if os.path.exists(self._sys_wireless_link_path):
                with open(self._sys_wireless_link_path, "r", encoding="ascii", errors="ignore") as f:
                    msg.wlan0_rssi = parse_wlan_rssi(f.read())
            else:
                msg.wlan0_rssi = -100
        except Exception:
            msg.wlan0_rssi = -100

        # 4. Interface IP & Netmask
        eth0_ip, eth0_mask = get_interface_ip_and_netmask("eth0")
        wlan0_ip, wlan0_mask = get_interface_ip_and_netmask("wlan0")
        ap_ip, ap_mask = get_interface_ip_and_netmask("uap0")

        msg.eth0_ip = eth0_ip[:16]
        msg.eth0_netmask = eth0_mask[:16]
        msg.wlan0_ip = wlan0_ip[:16]
        msg.wlan0_netmask = wlan0_mask[:16]
        msg.ap_ip = ap_ip[:16]
        msg.ap_netmask = ap_mask[:16]

        # 5. Interface status (0: down, 2: active)
        msg.eth0_status = get_interface_status("eth0")
        msg.wlan0_status = get_interface_status("wlan0")
        msg.ap_status = get_interface_status("uap0")

        # 6. Station SSID
        msg.wlan0_ssid = get_wlan_ssid("wlan0")[:32]

        # 7. Hotspot metadata
        act_path = self.get_parameter("active_hotspot_path").value
        def_path = self.get_parameter("default_wifi_path").value
        hotspot, _ = read_effective_hotspot(active_file=act_path, default_file=def_path)
        hw_mode, channel, _ = detect_frequency_channel(wlan_iface="wlan0")

        msg.ap_ssid = hotspot.get("ssid", "")[:32]
        msg.ap_channel = channel
        msg.ap_hw_mode = hw_mode[:4]
        msg.ap_key_mgmt = "WPA-PSK"
        msg.ap_ieee80211n = 1
        msg.ap_wmm_enabled = 1
        msg.ap_wpa = 2

        # 8. Extra status indicators
        msg.eth0_is_static = 1 if (eth0_ip.startswith("192.168.144.") or os.path.exists("/sys/class/net/eth0")) else 0
        msg.wlan0_dhcp = 1
        msg.dnsmasq_status = 2 if msg.ap_status == 2 else 0

        return msg

    def _publish_status(self):
        try:
            msg = self._collect_network_status()
            self._pub_status.publish(msg)
        except Exception as e:
            self.get_logger().error(f"Error publishing /cc/network_status: {e}")

    def _handle_reload_service(self, request, response):
        """
        Handle /cc/networking/reload service:
        - Validate effective hotspot credentials.
        - If invalid: response.success = False, do not exit.
        - If valid: response.success = True, schedule exit(3) for container restart.
        """
        act_path = self.get_parameter("active_hotspot_path").value
        def_path = self.get_parameter("default_wifi_path").value

        hotspot, src = read_effective_hotspot(active_file=act_path, default_file=def_path)
        ssid = hotspot.get("ssid", "")
        password = hotspot.get("password", "")

        if not hotspot.get("valid", True) or not is_valid_ssid(ssid) or not is_valid_wpa_passphrase(password):
            if not is_valid_ssid(ssid):
                response.message = f"Invalid SSID '{ssid}': must be 1-32 printable ASCII chars"
            else:
                response.message = "Invalid WPA2 passphrase: must be 8-63 printable ASCII chars"
            response.success = False
            self.get_logger().error(f"Reload rejected: {response.message}")
            return response

        response.success = True
        response.message = f"Hotspot configuration valid from {src}, restarting networking container via exit code 3"
        self.get_logger().info(response.message)

        # Trigger exit with code 3 asynchronously after response is sent
        def do_restart():
            time.sleep(0.3)
            self.get_logger().info("Exiting networking_node with code 3 for container restart...")
            self._exit_fn(3)

        threading.Thread(target=do_restart, daemon=True).start()
        return response


def main(args=None):
    rclpy.init(args=args)
    node = NetworkingNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
