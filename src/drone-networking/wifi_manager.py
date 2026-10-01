#!/usr/bin/env python3
"""
wifi_manager.py - Abstraction SDK & CLI for Drone WiFi Management.
Tuân thủ 5 nguyên lý SOLID:
- S: Quản lý duy nhất việc validate và cập nhật cấu hình WiFi (Data Contract).
- O: Mở rộng cho mọi chương trình gọi vào (ROS2, Web UI, CLI) mà không cần sửa code mạng bên dưới.
- L: Thay thế linh hoạt các provider lưu trữ khác nhau.
- I: Tách biệt giao diện Hotspot (phát) và Client (thu).
- D: Đảo ngược phụ thuộc, các chương trình phụ thuộc vào schema contract, không phụ thuộc hostapd/wpa_supplicant.
"""

import os
import sys
import json
import tempfile
import argparse
from abc import ABC, abstractmethod
from pathlib import Path


class WiFiConfigInterface(ABC):
    """Abstract Interface định nghĩa các thao tác quản lý WiFi."""

    @abstractmethod
    def get_config(self) -> dict:
        pass

    @abstractmethod
    def set_hotspot(self, ssid: str = None, password: str = None) -> bool:
        pass

    @abstractmethod
    def set_client(self, ssid: str = None, password: str = None) -> bool:
        pass


class JsonWiFiManager(WiFiConfigInterface):
    """Implementation chuẩn dựa trên File Data Contract JSON."""

    def __init__(self, config_path: str = None):
        if config_path:
            self.config_path = Path(config_path)
        elif os.environ.get("WIFI_CONFIG_PATH"):
            self.config_path = Path(os.environ["WIFI_CONFIG_PATH"])
        else:
            # Private runtime config; never write credentials to tracked source.
            project_root = Path(__file__).resolve().parent.parent.parent
            runtime_dir = Path(os.environ.get("DRONE_RUNTIME_DIR", "runtime"))
            if not runtime_dir.is_absolute():
                runtime_dir = project_root / runtime_dir
            candidates = [
                Path("/app/config/wifi.json"),
                runtime_dir / "networking" / "wifi.json",
            ]
            self.config_path = next((p for p in candidates if p.parent.exists()), candidates[1])

    def _load(self) -> dict:
        default_config = {
            "hotspot": {"ssid": "AP_DRONE", "password": "mypassword123"},
            "client": {"ssid": "YOUR_WIFI_SSID", "password": "change-this-client-password"},
        }
        if not self.config_path.exists():
            return default_config
        try:
            with open(self.config_path, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            print(f"[WiFiManager] Cảnh báo lỗi đọc file JSON, dùng mặc định: {e}", file=sys.stderr)
            return default_config

    def _save(self, data: dict) -> bool:
        """Ghi atomic bằng tempfile và replace để chống lỗi corrupt dữ liệu."""
        try:
            self.config_path.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.NamedTemporaryFile("w", dir=self.config_path.parent, delete=False, encoding="utf-8") as tf:
                json.dump(data, tf, indent=2)
                temp_name = tf.name
            os.replace(temp_name, self.config_path)
            return True
        except Exception as e:
            print(f"[WiFiManager] Lỗi khi lưu cấu hình WiFi: {e}", file=sys.stderr)
            return False

    @staticmethod
    def validate_password(password: str) -> bool:
        """Kiểm tra độ dài mật khẩu chuẩn WPA2-PSK (8 - 63 ký tự)."""
        if not password or len(password) < 8 or len(password) > 63:
            return False
        return True

    def get_config(self) -> dict:
        return self._load()

    def set_hotspot(self, ssid: str = None, password: str = None) -> bool:
        if password and not self.validate_password(password):
            raise ValueError("Mật khẩu Hotspot WPA2 phải có độ dài từ 8 đến 63 ký tự!")

        cfg = self._load()
        if "hotspot" not in cfg:
            cfg["hotspot"] = {}
        if ssid:
            cfg["hotspot"]["ssid"] = str(ssid).strip()
        if password:
            cfg["hotspot"]["password"] = str(password).strip()

        success = self._save(cfg)
        if success:
            print(f"✓ Đã cập nhật cấu hình Hotspot: SSID='{cfg['hotspot'].get('ssid')}'")
        return success

    def set_client(self, ssid: str = None, password: str = None) -> bool:
        if password and not self.validate_password(password):
            raise ValueError("Mật khẩu WiFi Client WPA2 phải có độ dài từ 8 đến 63 ký tự!")

        cfg = self._load()
        if "client" not in cfg:
            cfg["client"] = {}
        if ssid:
            cfg["client"]["ssid"] = str(ssid).strip()
        if password:
            cfg["client"]["password"] = str(password).strip()

        success = self._save(cfg)
        if success:
            print(f"✓ Đã cập nhật cấu hình WiFi Client: SSID='{cfg['client'].get('ssid')}'")
        return success


# Singleton instance để các chương trình khác import và gọi nhanh
WiFiManager = JsonWiFiManager()


def main():
    parser = argparse.ArgumentParser(description="Drone WiFi Manager SDK & CLI (SOLID Architecture)")
    subparsers = parser.add_subparsers(dest="command", help="Lệnh thao tác")

    # Lệnh show
    subparsers.add_parser("show", help="Hiển thị cấu hình WiFi hiện tại")

    # Lệnh set-hotspot
    p_ap = subparsers.add_parser("set-hotspot", help="Cập nhật thông tin Hotspot AP_DRONE")
    p_ap.add_argument("--ssid", type=str, help="Tên SSID mới của Hotspot")
    p_ap.add_argument("--password", "--pass", type=str, help="Mật khẩu mới (tối thiểu 8 ký tự)")

    # Lệnh set-client
    p_sta = subparsers.add_parser("set-client", help="Cập nhật thông tin kết nối WiFi ngoài")
    p_sta.add_argument("--ssid", type=str, help="Tên SSID WiFi ngoài")
    p_sta.add_argument("--password", "--pass", type=str, help="Mật khẩu WiFi ngoài (tối thiểu 8 ký tự)")

    args = parser.parse_args()
    mgr = JsonWiFiManager()

    if args.command == "show" or not args.command:
        cfg = mgr.get_config()
        print("==================================================")
        print("  📡 DRONE WIFI CONFIGURATION (DATA CONTRACT)")
        print("==================================================")
        print(f"  Hotspot SSID     : {cfg.get('hotspot', {}).get('ssid')}")
        print(f"  Hotspot Password : {'*' * len(cfg.get('hotspot', {}).get('password', ''))}")
        print("--------------------------------------------------")
        print(f"  Client WiFi SSID : {cfg.get('client', {}).get('ssid')}")
        print(f"  Client Password  : {'*' * len(cfg.get('client', {}).get('password', ''))}")
        print("==================================================")
    elif args.command == "set-hotspot":
        if not args.ssid and not args.password:
            print("Lỗi: Phải cung cấp ít nhất --ssid hoặc --password", file=sys.stderr)
            sys.exit(1)
        mgr.set_hotspot(ssid=args.ssid, password=args.password)
    elif args.command == "set-client":
        if not args.ssid and not args.password:
            print("Lỗi: Phải cung cấp ít nhất --ssid hoặc --password", file=sys.stderr)
            sys.exit(1)
        mgr.set_client(ssid=args.ssid, password=args.password)


if __name__ == "__main__":
    main()
