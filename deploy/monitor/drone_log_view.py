#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
drone_log_view.py -- Live Container & Telemetry Log Streamer
Xem log realtime từ các container và bộ lọc MAVLink QGC.

Usage:
    drone_log_view              Xem toàn bộ log thời gian thực
    drone_log_view <container>  Xem log lọc theo container (camera, mavlink, vision, networking)
    drone_log_view --once       Xem 30 log gần nhất rồi thoát (không stream)
"""
import sys
import subprocess
import re
import argparse

COLORS = {
    "CRIT": "\033[1;31m",
    "ERR":  "\033[0;31m",
    "WARN": "\033[0;33m",
    "INFO": "\033[0;32m",
    "RESET":"\033[0m",
    "SRC":  "\033[1;36m",
}

FILTER_MAP = {
    "camera":  "camera-stream-controller",
    "cam":     "camera-stream-controller",
    "rtsp":    "camera-stream-controller",
    "mavlink": "mavlink-router-controller",
    "router":  "mavlink-router-controller",
    "vision":  "drone-vision",
    "vis":     "drone-vision",
    "yolo":    "drone-vision",
    "network": "drone-networking",
    "net":     "drone-networking",
    "wifi":    "drone-networking",
    "mavros":  "mavros",
}

def main():
    parser = argparse.ArgumentParser(description="Live Container & Telemetry Log Streamer")
    parser.add_argument("target", nargs="?", default="", help="Tên container hoặc module (camera, mavlink, vision, networking...)")
    parser.add_argument("--once", action="store_true", help="Chỉ xem log gần nhất rồi thoát")
    parser.add_argument("--tail", type=int, default=30, help="Số dòng log hiển thị (mặc định 30)")
    args = parser.parse_args()

    target = args.target.lower().strip()
    if target in FILTER_MAP:
        target = FILTER_MAP[target]

    print("==========================================================================")
    print("🛰️ THACO DRONE LOG STREAMER (Live QGC StatusText & Container Logs)")
    if target:
        print(f"🔍 Bộ lọc container: [{target}]")
    else:
        print("🔍 Đang xem: TOÀN BỘ CONTAINERS")
    print("==========================================================================")

    cmd = ["docker", "compose", "--profile", "mavros", "logs"]
    if not args.once:
        cmd.append("-f")
    cmd.extend(["--tail", str(args.tail), "mavros"])

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)

    try:
        for line in iter(proc.stdout.readline, ""):
            line = line.strip()
            if not line:
                continue
            if "[QGC-log]" not in line and "sev=" not in line and "[cc_telemetry]" not in line:
                continue

            # Check filter
            if target and target.lower() not in line.lower() and not (target == "mavros" and "cc_telemetry" in line.lower()):
                continue

            # Colorize output
            color = COLORS["INFO"]
            if "sev=2" in line or "CRIT" in line:
                color = COLORS["CRIT"]
            elif "sev=3" in line or "ERR" in line:
                color = COLORS["ERR"]
            elif "sev=4" in line or "WARN" in line:
                color = COLORS["WARN"]

            # Highlight source tag
            formatted = re.sub(r"(\[[^\]]+\])", rf"{COLORS['SRC']}\1{COLORS['RESET']}{color}", line)
            print(f"{color}{formatted}{COLORS['RESET']}")
    except KeyboardInterrupt:
        pass
    finally:
        try:
            proc.terminate()
        except Exception:
            pass

if __name__ == "__main__":
    main()
