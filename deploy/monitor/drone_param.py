#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
drone_param.py -- Runtime Telemetry Parameter Controller
Quản lý, chỉnh sửa và bật/tắt các tham số telemetry tức thì khi container đang chạy.

Usage:
    drone_param list                  Liệt kê toàn bộ tham số, giá trị và trạng thái
    drone_param get <PARAM_NAME>      Đọc giá trị tham số
    drone_param set <PARAM> <VAL>     Đặt giá trị tham số (live update tức thì)
    drone_param on  <GROUP>           Bật nhóm telemetry (links, camera, network, vision, qgc, banner)
    drone_param off <GROUP>           Tắt nhóm telemetry (links, camera, network, vision, qgc, banner)
"""
import sys
import os
import json
import socket
import select

SOCKET_PATH = os.environ.get("DRONE_LOG_SOCK", "/run/drone/log.sock")
CLIENT_SOCK_DIR = os.environ.get("DRONE_RUN_DIR", "/run/drone")

GROUP_MAP = {
    "links":   "CC_LINK_HZ",
    "link":    "CC_LINK_HZ",
    "camera":  "CC_CAM_HZ",
    "cam":     "CC_CAM_HZ",
    "network": "CC_NET_HZ",
    "net":     "CC_NET_HZ",
    "vision":  "CC_VIS_HZ",
    "vis":     "CC_VIS_HZ",
    "qgc":     "CC_QGC_TEL",
    "banner":  "CC_QGC_BANNER",
}

DEFAULT_ON_RATES = {
    "CC_LINK_HZ":     1.0,
    "CC_CAM_HZ":      1.0,
    "CC_NET_HZ":      0.5,
    "CC_VIS_HZ":      1.0,
    "CC_QGC_TEL":     1.0,
    "CC_QGC_BANNER":  1.0,
}

def _send_cmd(payload: dict, timeout: float = 2.5) -> dict:
    client_sock = f"{CLIENT_SOCK_DIR}/param_client_{os.getpid()}.sock"
    if os.path.exists(client_sock):
        try: os.unlink(client_sock)
        except OSError: pass

    s = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    try:
        s.bind(client_sock)
        os.chmod(client_sock, 0o777)
    except Exception as e:
        return {"ok": False, "error": f"Cannot bind client socket at {client_sock}: {e}"}

    s.setblocking(False)
    try:
        data = json.dumps(payload).encode("utf-8")
        s.sendto(data, SOCKET_PATH)

        r, _, _ = select.select([s], [], [], timeout)
        if not r:
            return {"ok": False, "error": "Timeout waiting for cc_telemetry_node response"}
        reply_data, _ = s.recvfrom(4096)
        return json.loads(reply_data.decode("utf-8"))
    except Exception as e:
        return {"ok": False, "error": str(e)}
    finally:
        s.close()
        if os.path.exists(client_sock):
            try: os.unlink(client_sock)
            except OSError: pass

def cmd_list():
    res = _send_cmd({"cmd": "get_params"})
    if not res or "error" in res:
        err = res.get("error", "Unknown error") if res else "No response"
        print(f"❌ Lỗi kết nối tới cc_telemetry: {err}")
        sys.exit(1)

    print("=" * 74)
    print("🚁 THACO AGRI-DRONE TELEMETRY PARAMETERS (LIVE RUNTIME)")
    print("=" * 74)
    print(f"{'Parameter':<16} {'Value':<8} {'Active':<8} {'Min..Max':<14} {'Status'}")
    print("-" * 74)

    for name in sorted(res.keys()):
        p = res[name]
        val = p.get("val", 0.0)
        act = p.get("active", val)
        p_min = p.get("min", 0.0)
        p_max = p.get("max", 0.0)
        
        status = ""
        if name.endswith("_HZ"):
            status = f"🟢 ON ({act:.1f} Hz)" if act > 0.0 else "🔴 OFF (0.0 Hz)"
        elif name in ("CC_QGC_TEL", "CC_QGC_BANNER"):
            status = "🟢 BẬT" if act > 0.5 else "🔴 TẮT"
        elif name == "CC_LOG_SEV":
            sev_names = {2: "CRITICAL", 3: "ERROR", 4: "WARNING", 5: "NOTICE", 6: "INFO"}
            status = f"Mức log: {sev_names.get(int(act), str(int(act)))}"
        elif name == "CC_CAM_KBPS":
            status = f"{int(act)} kbps"
        elif name == "CC_CAM_FPS":
            status = f"{int(act)} fps"
        elif name == "CC_YOLO_CONF":
            status = f"Conf: {act:.2f}"

        print(f"{name:<16} {val:<8.2f} {act:<8.2f} [{p_min:.1f}..{p_max:.1f}]   {status}")
    print("=" * 74)

def cmd_get(pname: str):
    res = _send_cmd({"cmd": "get_params"})
    if not res or "error" in res:
        err = res.get("error", "Unknown error") if res else "No response"
        print(f"❌ Lỗi: {err}")
        sys.exit(1)
    
    pname = pname.upper().strip()
    if pname in res:
        p = res[pname]
        print(f"{pname} = {p.get('val', 0.0):.2f} (active: {p.get('active', 0.0):.2f}, min: {p.get('min')}, max: {p.get('max')})")
    else:
        print(f"❌ Không tìm thấy tham số [{pname}]. Chạy 'drone_param list' để xem danh sách.")
        sys.exit(1)

def cmd_set(pname: str, val: float):
    pname = pname.upper().strip()
    res = _send_cmd({"cmd": "set_param", "name": pname, "val": val})
    if res.get("ok"):
        reason = res.get("reason", "applied")
        print(f"✓ [{pname}] cập nhật thành công: {val} ({reason})")
    else:
        err = res.get("reason") or res.get("error") or "unknown error"
        print(f"❌ Không thể đặt [{pname}]: {err}")
        sys.exit(1)

def cmd_toggle(name: str, enable: bool):
    key = name.lower().strip()
    if key not in GROUP_MAP:
        print(f"❌ Nhóm '{name}' không hợp lệ. Các nhóm hỗ trợ: {', '.join(sorted(set(GROUP_MAP.keys())))}")
        sys.exit(1)

    pname = GROUP_MAP[key]
    if enable:
        rate = DEFAULT_ON_RATES.get(pname, 1.0)
        cmd_set(pname, rate)
        print(f"🟢 Đã BẬT nhóm [{key}] với tần số {rate} Hz")
    else:
        cmd_set(pname, 0.0)
        print(f"🔴 Đã TẮT nhóm [{key}] (tần số 0.0 Hz)")

def print_help():
    print(__doc__.strip())

def main():
    if len(sys.argv) < 2:
        print_help()
        sys.exit(0)

    action = sys.argv[1].lower().strip()
    if action in ("list", "ls"):
        cmd_list()
    elif action == "get" and len(sys.argv) >= 3:
        cmd_get(sys.argv[2])
    elif action == "set" and len(sys.argv) >= 4:
        try:
            val = float(sys.argv[3])
        except ValueError:
            print(f"❌ Giá trị '{sys.argv[3]}' không hợp lệ (phải là số).")
            sys.exit(1)
        cmd_set(sys.argv[2], val)
    elif action == "on" and len(sys.argv) >= 3:
        cmd_toggle(sys.argv[2], True)
    elif action == "off" and len(sys.argv) >= 3:
        cmd_toggle(sys.argv[2], False)
    else:
        print_help()
        sys.exit(1)

if __name__ == "__main__":
    main()
