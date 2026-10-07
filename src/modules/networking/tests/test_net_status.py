"""Unit tests for net_status.py."""
import unittest

from networking.net_status import (
    parse_net_dev,
    parse_arp,
    parse_wlan_rssi,
    get_interface_ip_and_netmask,
    get_interface_status,
)

SAMPLE_PROC_NET_DEV = """Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    lo: 10485760    1000    0    0    0     0          0         0 10485760    1000    0    0    0     0       0          0
  eth0:  2097152     200    0    0    0     0          0         0  4194304     400    0    0    0     0       0          0
 wlan0:   524288      50    0    0    0     0          0         0  1048576     100    0    0    0     0       0          0
  uap0:   262144      25    0    0    0     0          0         0   524288      50    0    0    0     0       0          0
"""

SAMPLE_PROC_NET_ARP = """IP address       HW type     Flags       HW address            Mask     Device
192.168.10.15    0x1         0x2         aa:bb:cc:dd:ee:ff     *        uap0
192.168.10.20    0x1         0x0         00:00:00:00:00:00     *        uap0
10.14.80.1       0x1         0x2         11:22:33:44:55:66     *        eth0
"""


class NetStatusTest(unittest.TestCase):
    def test_parse_net_dev(self):
        stats = parse_net_dev(SAMPLE_PROC_NET_DEV)
        self.assertIn("eth0", stats)
        self.assertIn("wlan0", stats)
        self.assertIn("uap0", stats)

        # eth0: rx=2097152 bytes = 2048 KB, tx=4194304 bytes = 4096 KB
        self.assertEqual(stats["eth0"]["rx_bytes"], 2097152)
        self.assertEqual(stats["eth0"]["rx_kb"], 2048)
        self.assertEqual(stats["eth0"]["tx_bytes"], 4194304)
        self.assertEqual(stats["eth0"]["tx_kb"], 4096)

        # uap0: rx=262144 = 256 KB, tx=524288 = 512 KB
        self.assertEqual(stats["uap0"]["rx_kb"], 256)
        self.assertEqual(stats["uap0"]["tx_kb"], 512)

    def test_parse_arp(self):
        # 192.168.10.15 is valid, 192.168.10.20 has zero MAC, 10.14.80.1 is different subnet
        count = parse_arp(SAMPLE_PROC_NET_ARP, subnet_prefix="192.168.10.")
        self.assertEqual(count, 1)

    def test_parse_wlan_rssi(self):
        # 45 quality -> -100 + 45 = -55 dBm
        self.assertEqual(parse_wlan_rssi("45\n"), -55)
        # None or invalid -> -100
        self.assertEqual(parse_wlan_rssi(None), -100)
        self.assertEqual(parse_wlan_rssi("not-a-number"), -100)

    def test_get_interface_ip_and_netmask_lo(self):
        ip, mask = get_interface_ip_and_netmask("lo")
        self.assertEqual(ip, "127.0.0.1")
        self.assertEqual(mask, "255.0.0.0")

    def test_get_interface_ip_nonexistent(self):
        ip, mask = get_interface_ip_and_netmask("nonexistent_if99")
        self.assertEqual(ip, "")
        self.assertEqual(mask, "")

    def test_get_interface_status_lo(self):
        status = get_interface_status("lo")
        self.assertEqual(status, 2)  # Active / UP

    def test_get_interface_status_nonexistent(self):
        status = get_interface_status("nonexistent_if99")
        self.assertEqual(status, 0)


if __name__ == "__main__":
    unittest.main()
