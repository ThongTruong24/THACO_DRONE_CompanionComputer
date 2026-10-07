"""
hotspot_config.py - Dynamic generation and validation of hostapd configuration for Drone WiFi AP.
Ports PC Task 18 without using sed, supporting AP_COUNTRY, frequency auto-adaptation, and active/default configs.
"""
import argparse
import json
import os
import re
import subprocess
import sys
from typing import Optional, Tuple


def is_valid_ssid(ssid: str) -> bool:
    """Validate SSID: 1 to 32 printable ASCII characters."""
    if not isinstance(ssid, str):
        return False
    if not (1 <= len(ssid) <= 32):
        return False
    return all(0x20 <= ord(c) <= 0x7E for c in ssid)


def is_valid_wpa_passphrase(password: str) -> bool:
    """Validate WPA2-PSK passphrase: 8 to 63 printable ASCII characters."""
    if not isinstance(password, str):
        return False
    if not (8 <= len(password) <= 63):
        return False
    return all(0x20 <= ord(c) <= 0x7E for c in password)


def read_effective_hotspot(
    active_file: str = "/run/drone/hotspot.json",
    default_file: str = "config/wifi.json",
    env_ssid: Optional[str] = None,
    env_password: Optional[str] = None,
) -> Tuple[dict, str]:
    """
    Read effective hotspot credentials following priority:
    1. active_file (/run/drone/hotspot.json)
    2. default_file (config/wifi.json or /app/config/wifi.json)
    3. env vars (AP_SSID, AP_PASS)
    4. fallback ("AP_DRONE", "mypassword123")
    
    Returns ({"ssid": ssid, "password": password}, source_description).
    """
    # 1. Try active file
    if os.path.exists(active_file):
        try:
            with open(active_file, "r", encoding="utf-8") as f:
                data = json.load(f)
            ssid = data.get("ssid", "")
            pw = data.get("password", "")
            valid = is_valid_ssid(ssid) and is_valid_wpa_passphrase(pw)
            return {"ssid": ssid, "password": pw, "valid": valid}, f"active:{active_file}"
        except Exception:
            return {"ssid": "", "password": "", "valid": False}, f"active_corrupt:{active_file}"

    # 2. Try default file
    candidates = [default_file, "/app/config/wifi.json", "config/wifi.json"]
    for path in candidates:
        if os.path.exists(path):
            try:
                with open(path, "r", encoding="utf-8") as f:
                    data = json.load(f)
                h = data.get("hotspot")
                if h is not None and isinstance(h, dict):
                    ssid = h.get("ssid", "")
                    pw = h.get("password", "")
                    valid = is_valid_ssid(ssid) and is_valid_wpa_passphrase(pw)
                    return {"ssid": ssid, "password": pw, "valid": valid}, f"file:{path}"
            except Exception:
                return {"ssid": "", "password": "", "valid": False}, f"file_corrupt:{path}"

    # 3. Environment variables
    e_ssid = env_ssid or os.environ.get("AP_SSID") or os.environ.get("WIFI_AP_SSID")
    e_pw = env_password or os.environ.get("AP_PASS") or os.environ.get("WIFI_AP_PASSWORD")
    if e_ssid or e_pw:
        valid = is_valid_ssid(e_ssid or "") and is_valid_wpa_passphrase(e_pw or "")
        return {"ssid": e_ssid or "", "password": e_pw or "", "valid": valid}, "env"

    # 4. Builtin safe fallback
    return {"ssid": "AP_DRONE", "password": "mypassword123", "valid": True}, "default"


def detect_frequency_channel(
    wlan_iface: str = "wlan0",
    fallback_freq: Optional[int] = None,
) -> Tuple[str, int, bool]:
    """
    Detect upstream frequency on wlan_iface to avoid inter-channel interference
    when running concurrent AP-STA on a single radio.
    
    Returns (hw_mode, channel, is_5g).
    """
    freq = fallback_freq
    if freq is None:
        try:
            out = subprocess.check_output(
                ["iw", "dev", wlan_iface, "link"],
                stderr=subprocess.DEVNULL,
                timeout=2.0,
                text=True,
            )
            match = re.search(r"freq:\s*(\d+)", out)
            if match:
                freq = int(match.group(1))
        except Exception:
            freq = None

    if freq is not None:
        if freq >= 5000:
            channel = (freq - 5000) // 5
            return ("a", channel, True)
        elif freq >= 2400:
            if freq == 2484:
                channel = 14
            else:
                channel = (freq - 2407) // 5
            return ("g", channel, False)

    # Fallback default: 2.4 GHz channel 6
    return ("g", 6, False)


def generate_hostapd_conf(
    ssid: str,
    password: str,
    hw_mode: str = "g",
    channel: int = 6,
    is_5g: bool = False,
    ap_iface: str = "uap0",
    country_code: str = "US",
) -> str:
    """Generate hostapd.conf content cleanly without sed or external tools."""
    if not is_valid_ssid(ssid):
        raise ValueError(f"Invalid SSID: {ssid!r}")
    if not is_valid_wpa_passphrase(password):
        raise ValueError("Invalid WPA2 passphrase (must be 8-63 printable chars)")

    lines = [
        f"interface={ap_iface}",
        "driver=nl80211",
        f"ssid={ssid}",
        f"hw_mode={hw_mode}",
        f"channel={channel}",
        "wmm_enabled=1",
        "auth_algs=1",
        "ignore_broadcast_ssid=0",
        "",
        "# WPA2-PSK Security",
        "wpa=2",
        f"wpa_passphrase={password}",
        "wpa_key_mgmt=WPA-PSK",
        "wpa_pairwise=CCMP",
        "rsn_pairwise=CCMP",
    ]

    if is_5g:
        lines.extend([
            f"country_code={country_code}",
            "ieee80211d=1",
            "ieee80211n=1",
            "ieee80211ac=1",
        ])
    else:
        lines.append("ieee80211n=1")

    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description="Generate hostapd configuration")
    parser.add_argument("--output", "-o", default=None, help="Output file path (default: stdout)")
    parser.add_argument("--active-file", default="/run/drone/hotspot.json", help="Active hotspot JSON file")
    parser.add_argument("--default-file", default="/app/config/wifi.json", help="Default wifi JSON file")
    parser.add_argument("--wlan-iface", default=os.environ.get("WIFI_PARENT", "wlan0"), help="Parent WLAN interface")
    parser.add_argument("--ap-iface", default=os.environ.get("AP_IFACE", "uap0"), help="AP virtual interface")
    parser.add_argument("--country", default=os.environ.get("AP_COUNTRY", "US"), help="Country code for regulatory domain")
    parser.add_argument("--selftest", action="store_true", help="Run self-test validation")
    args = parser.parse_args()

    if args.selftest:
        conf = generate_hostapd_conf("TEST_SSID", "testpassword123", "g", 6, False)
        assert "ssid=TEST_SSID" in conf
        assert "wpa_passphrase=testpassword123" in conf
        assert "ieee80211n=1" in conf
        print("[hotspot_config] Self-test passed successfully.")
        return 0

    hotspot, src = read_effective_hotspot(active_file=args.active_file, default_file=args.default_file)
    hw_mode, channel, is_5g = detect_frequency_channel(wlan_iface=args.wlan_iface)

    conf_text = generate_hostapd_conf(
        ssid=hotspot["ssid"],
        password=hotspot["password"],
        hw_mode=hw_mode,
        channel=channel,
        is_5g=is_5g,
        ap_iface=args.ap_iface,
        country_code=args.country,
    )

    if args.output:
        os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(conf_text)
        print(f"[hotspot_config] Generated hostapd configuration from {src} -> {args.output}")
    else:
        sys.stdout.write(conf_text)

    return 0


if __name__ == "__main__":
    sys.exit(main())
