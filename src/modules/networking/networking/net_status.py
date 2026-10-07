"""
net_status.py - Network monitoring functions for THACO Drone.
Ports network_monitor.cpp:
- Reads rx/tx counters from /proc/net/dev
- Counts connected AP clients on uap0 from /proc/net/arp
- Reads wlan0 RSSI from /sys/class/net/wlan0/wireless/link
- Queries interface IP, netmask, and status via standard ioctl
"""
import fcntl
import os
import socket
import struct
import subprocess
from typing import Dict, Optional, Tuple

SIOCGIFADDR = 0x8915
SIOCGIFNETMASK = 0x891B
SIOCGIFFLAGS = 0x8913
IFF_UP = 0x1


def parse_net_dev(content: str) -> Dict[str, Dict[str, int]]:
    """
    Parse /proc/net/dev content into per-interface statistics in KB and bytes.
    Returns: {iface: {"rx_bytes": int, "tx_bytes": int, "rx_kb": int, "tx_kb": int}}
    """
    stats = {}
    lines = content.strip().splitlines()
    for line in lines:
        if ":" not in line:
            continue
        parts = line.split(":", 1)
        iface = parts[0].strip()
        data = parts[1].split()
        if len(data) >= 9:
            try:
                rx_bytes = int(data[0])
                tx_bytes = int(data[8])
                stats[iface] = {
                    "rx_bytes": rx_bytes,
                    "tx_bytes": tx_bytes,
                    "rx_kb": rx_bytes // 1024,
                    "tx_kb": tx_bytes // 1024,
                }
            except ValueError:
                continue
    return stats


def parse_arp(content: str, subnet_prefix: str = "192.168.10.") -> int:
    """
    Count connected clients in AP subnet from /proc/net/arp.
    Only counts entries matching subnet_prefix with non-zero MAC.
    """
    count = 0
    lines = content.strip().splitlines()
    if not lines:
        return 0
    # Skip header
    for line in lines[1:]:
        if subnet_prefix in line and "00:00:00:00:00:00" not in line:
            count += 1
    return count


def parse_wlan_rssi(sys_link_content: Optional[str] = None) -> int:
    """
    Parse WLAN RSSI from /sys/class/net/<iface>/wireless/link.
    Link quality is typically 0-70, mapped to approx -100 to -30 dBm:
    rssi = -100 + quality.
    Returns int8 clamped between -100 and 0.
    """
    if sys_link_content is None:
        return -100
    try:
        val = int(sys_link_content.strip())
        rssi = -100 + val
        return max(-100, min(0, rssi))
    except (ValueError, TypeError):
        return -100


def get_interface_ip_and_netmask(iface: str) -> Tuple[str, str]:
    """Retrieve IPv4 address and netmask for interface via ioctl."""
    ip = ""
    netmask = ""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        iface_b = iface[:15].encode("ascii")
        ifr = struct.pack("256s", iface_b)

        # IP
        try:
            res_ip = fcntl.ioctl(s.fileno(), SIOCGIFADDR, ifr)
            ip = socket.inet_ntoa(res_ip[20:24])
        except OSError:
            pass

        # Netmask
        try:
            res_mask = fcntl.ioctl(s.fileno(), SIOCGIFNETMASK, ifr)
            netmask = socket.inet_ntoa(res_mask[20:24])
        except OSError:
            pass

        s.close()
    except Exception:
        pass

    return ip, netmask


def get_interface_status(iface: str) -> int:
    """
    Retrieve interface status code:
    0: INACTIVE/DOWN
    2: ACTIVE/UP (matches network_monitor.cpp status code 2)
    """
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        iface_b = iface[:15].encode("ascii")
        ifr = struct.pack("256s", iface_b)
        res = fcntl.ioctl(s.fileno(), SIOCGIFFLAGS, ifr)
        flags = struct.unpack("H", res[16:18])[0]
        s.close()
        return 2 if (flags & IFF_UP) else 0
    except Exception:
        return 0


def get_wlan_ssid(iface: str = "wlan0") -> str:
    """Read current connected SSID on wlan0 using iw if available."""
    try:
        out = subprocess.check_output(
            ["iw", "dev", iface, "link"],
            stderr=subprocess.DEVNULL,
            timeout=1.0,
            text=True,
        )
        for line in out.splitlines():
            line = line.strip()
            if line.startswith("SSID: "):
                return line[6:].strip()
    except Exception:
        pass
    return ""
