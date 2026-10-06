"""Unit tests for hotspot_config.py."""
import json
import os
import shutil
import tempfile
import unittest

from networking.hotspot_config import (
    is_valid_ssid,
    is_valid_wpa_passphrase,
    detect_frequency_channel,
    generate_hostapd_conf,
    read_effective_hotspot,
)


class HotspotConfigTest(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.mkdtemp(prefix="test_hotspot_")

    def tearDown(self):
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_ssid_validation(self):
        self.assertTrue(is_valid_ssid("AP_DRONE"))
        self.assertTrue(is_valid_ssid("A"))
        self.assertTrue(is_valid_ssid("X" * 32))

        self.assertFalse(is_valid_ssid(""))
        self.assertFalse(is_valid_ssid("X" * 33))
        self.assertFalse(is_valid_ssid("SSID\x00NULL"))
        self.assertFalse(is_valid_ssid(None))

    def test_wpa_passphrase_validation(self):
        self.assertTrue(is_valid_wpa_passphrase("12345678"))
        self.assertTrue(is_valid_wpa_passphrase("mypassword123"))
        self.assertTrue(is_valid_wpa_passphrase("P" * 63))

        self.assertFalse(is_valid_wpa_passphrase("1234567"))
        self.assertFalse(is_valid_wpa_passphrase("P" * 64))
        self.assertFalse(is_valid_wpa_passphrase("pass\nword"))
        self.assertFalse(is_valid_wpa_passphrase(None))

    def test_frequency_channel_detection(self):
        # 5 GHz: 5180 MHz -> channel (5180 - 5000) / 5 = 36
        mode, ch, is_5g = detect_frequency_channel(fallback_freq=5180)
        self.assertEqual(mode, "a")
        self.assertEqual(ch, 36)
        self.assertTrue(is_5g)

        # 2.4 GHz: 2437 MHz -> channel (2437 - 2407) / 5 = 6
        mode, ch, is_5g = detect_frequency_channel(fallback_freq=2437)
        self.assertEqual(mode, "g")
        self.assertEqual(ch, 6)
        self.assertFalse(is_5g)

        # 2.4 GHz special channel 14
        mode, ch, is_5g = detect_frequency_channel(fallback_freq=2484)
        self.assertEqual(mode, "g")
        self.assertEqual(ch, 14)
        self.assertFalse(is_5g)

        # Fallback
        mode, ch, is_5g = detect_frequency_channel(fallback_freq=None)
        self.assertEqual(mode, "g")
        self.assertEqual(ch, 6)
        self.assertFalse(is_5g)

    def test_generate_hostapd_conf_2g(self):
        conf = generate_hostapd_conf(
            ssid="MY_DRONE_AP",
            password="securepassword123",
            hw_mode="g",
            channel=6,
            is_5g=False,
            ap_iface="uap0",
        )
        self.assertIn("interface=uap0", conf)
        self.assertIn("ssid=MY_DRONE_AP", conf)
        self.assertIn("wpa_passphrase=securepassword123", conf)
        self.assertIn("hw_mode=g", conf)
        self.assertIn("channel=6", conf)
        self.assertIn("ieee80211n=1", conf)
        self.assertNotIn("ieee80211ac=1", conf)

    def test_generate_hostapd_conf_5g(self):
        conf = generate_hostapd_conf(
            ssid="MY_DRONE_5G",
            password="securepassword123",
            hw_mode="a",
            channel=36,
            is_5g=True,
            ap_iface="uap0",
            country_code="VN",
        )
        self.assertIn("interface=uap0", conf)
        self.assertIn("ssid=MY_DRONE_5G", conf)
        self.assertIn("hw_mode=a", conf)
        self.assertIn("channel=36", conf)
        self.assertIn("country_code=VN", conf)
        self.assertIn("ieee80211ac=1", conf)

    def test_generate_hostapd_conf_invalid_credentials(self):
        with self.assertRaises(ValueError):
            generate_hostapd_conf("", "password123")
        with self.assertRaises(ValueError):
            generate_hostapd_conf("SSID", "short")

    def test_read_effective_hotspot_active_priority(self):
        active_path = os.path.join(self.test_dir, "hotspot.json")
        default_path = os.path.join(self.test_dir, "wifi.json")

        with open(active_path, "w") as f:
            json.dump({"ssid": "ACTIVE_AP", "password": "activepassword1"}, f)

        with open(default_path, "w") as f:
            json.dump({"hotspot": {"ssid": "DEFAULT_AP", "password": "defaultpassword1"}}, f)

        res, src = read_effective_hotspot(active_file=active_path, default_file=default_path)
        self.assertEqual(res["ssid"], "ACTIVE_AP")
        self.assertEqual(res["password"], "activepassword1")
        self.assertTrue(src.startswith("active:"))

    def test_read_effective_hotspot_fallback_to_default(self):
        active_path = os.path.join(self.test_dir, "nonexistent.json")
        default_path = os.path.join(self.test_dir, "wifi.json")

        with open(default_path, "w") as f:
            json.dump({"hotspot": {"ssid": "DEFAULT_AP", "password": "defaultpassword1"}}, f)

        res, src = read_effective_hotspot(active_file=active_path, default_file=default_path)
        self.assertEqual(res["ssid"], "DEFAULT_AP")
        self.assertEqual(res["password"], "defaultpassword1")
        self.assertTrue(src.startswith("file:"))


if __name__ == "__main__":
    unittest.main()
