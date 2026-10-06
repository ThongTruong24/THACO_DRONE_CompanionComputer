"""Unit tests for networking_node.py."""
import json
import os
import shutil
import tempfile
import time
import unittest

import rclpy
from std_srvs.srv import Trigger

from networking.networking_node import NetworkingNode

SAMPLE_PROC_NET_DEV = """Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
  eth0:  2097152     200    0    0    0     0          0         0  4194304     400    0    0    0     0       0          0
 wlan0:   524288      50    0    0    0     0          0         0  1048576     100    0    0    0     0       0          0
  uap0:   262144      25    0    0    0     0          0         0   524288      50    0    0    0     0       0          0
"""

SAMPLE_PROC_NET_ARP = """IP address       HW type     Flags       HW address            Mask     Device
192.168.10.15    0x1         0x2         aa:bb:cc:dd:ee:ff     *        uap0
"""


class NetworkingNodeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        rclpy.init()

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()

    def setUp(self):
        self.test_dir = tempfile.mkdtemp(prefix="test_node_")
        self.dev_path = os.path.join(self.test_dir, "proc_net_dev")
        with open(self.dev_path, "w") as f:
            f.write(SAMPLE_PROC_NET_DEV)

        self.arp_path = os.path.join(self.test_dir, "proc_net_arp")
        with open(self.arp_path, "w") as f:
            f.write(SAMPLE_PROC_NET_ARP)

        self.active_hotspot_path = os.path.join(self.test_dir, "hotspot.json")
        self.default_wifi_path = os.path.join(self.test_dir, "wifi.json")

        self.exit_code = None

    def tearDown(self):
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def _mock_exit(self, code):
        self.exit_code = code

    def test_collect_network_status(self):
        with open(self.active_hotspot_path, "w") as f:
            json.dump({"ssid": "TEST_SSID_123", "password": "valid_pass_123"}, f)

        node = NetworkingNode(
            node_name="test_net_node_1",
            exit_fn=self._mock_exit,
            proc_net_dev_path=self.dev_path,
            proc_net_arp_path=self.arp_path,
        )
        node.set_parameters([
            rclpy.Parameter("active_hotspot_path", rclpy.Parameter.Type.STRING, self.active_hotspot_path),
            rclpy.Parameter("default_wifi_path", rclpy.Parameter.Type.STRING, self.default_wifi_path),
        ])

        msg = node._collect_network_status()
        self.assertGreater(msg.timestamp, 0)
        self.assertEqual(msg.uap0_rx_kb, 256)
        self.assertEqual(msg.uap0_tx_kb, 512)
        self.assertEqual(msg.eth0_rx_kb, 2048)
        self.assertEqual(msg.eth0_tx_kb, 4096)
        self.assertEqual(msg.ap_client_count, 1)
        self.assertEqual(msg.ap_ssid, "TEST_SSID_123")
        self.assertEqual(msg.ap_wpa, 2)
        self.assertEqual(msg.ap_ieee80211n, 1)
        node.destroy_node()

    def test_reload_service_valid_config(self):
        with open(self.active_hotspot_path, "w") as f:
            json.dump({"ssid": "VALID_SSID", "password": "valid_password_123"}, f)

        node = NetworkingNode(
            node_name="test_net_node_2",
            exit_fn=self._mock_exit,
            proc_net_dev_path=self.dev_path,
            proc_net_arp_path=self.arp_path,
        )
        node.set_parameters([
            rclpy.Parameter("active_hotspot_path", rclpy.Parameter.Type.STRING, self.active_hotspot_path),
            rclpy.Parameter("default_wifi_path", rclpy.Parameter.Type.STRING, self.default_wifi_path),
        ])

        req = Trigger.Request()
        resp = Trigger.Response()
        result_resp = node._handle_reload_service(req, resp)

        self.assertTrue(result_resp.success)
        self.assertIn("exit code 3", result_resp.message)

        # Wait a short moment for the background thread to call mock exit
        time.sleep(0.5)
        self.assertEqual(self.exit_code, 3)
        node.destroy_node()

    def test_reload_service_invalid_config(self):
        # Invalid password (less than 8 chars)
        with open(self.active_hotspot_path, "w") as f:
            json.dump({"ssid": "VALID_SSID", "password": "short"}, f)

        node = NetworkingNode(
            node_name="test_net_node_3",
            exit_fn=self._mock_exit,
            proc_net_dev_path=self.dev_path,
            proc_net_arp_path=self.arp_path,
        )
        node.set_parameters([
            rclpy.Parameter("active_hotspot_path", rclpy.Parameter.Type.STRING, self.active_hotspot_path),
            rclpy.Parameter("default_wifi_path", rclpy.Parameter.Type.STRING, self.default_wifi_path),
        ])

        req = Trigger.Request()
        resp = Trigger.Response()
        result_resp = node._handle_reload_service(req, resp)

        self.assertFalse(result_resp.success)
        self.assertIn("Invalid WPA2 passphrase", result_resp.message)

        time.sleep(0.4)
        self.assertIsNone(self.exit_code)  # Must NOT exit on invalid config!
        node.destroy_node()


if __name__ == "__main__":
    unittest.main()
