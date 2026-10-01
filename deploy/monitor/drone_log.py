"""
drone_log.py -- QGC Log Aggregation Client
Gui log tu bat ky container nao den cc_telemetry_node qua UNIX socket
/run/drone/log.sock, node se forward len QGroundControl qua MAVLink STATUSTEXT.

Usage (Python):
    from drone_log import DroneLogger
    log = DroneLogger("drone-mavlink")
    log.info("MAVLink router started")
    log.warning("GCS IP not set, broadcast mode")
    log.error("Serial port open failed")
    log.critical("Router crashed")

Usage (CLI test):
    python3 /tmp/drone_log.py --src drone-test --sev info --msg "hello QGC"

MAVLink severity: CRITICAL=2(red bold), ERROR=3(red), WARNING=4(yellow), INFO=6(white)
"""

from __future__ import annotations
import json
import os
import socket
import time

_SOCKET_PATH = os.environ.get("DRONE_LOG_SOCK", "/run/drone/log.sock")

SEV_CRITICAL = 2
SEV_ERROR    = 3
SEV_WARNING  = 4
SEV_INFO     = 6

_SEV_MAP = {
    "critical": SEV_CRITICAL,
    "error":    SEV_ERROR,
    "warning":  SEV_WARNING,
    "warn":     SEV_WARNING,
    "info":     SEV_INFO,
    "2":        SEV_CRITICAL,
    "3":        SEV_ERROR,
    "4":        SEV_WARNING,
    "6":        SEV_INFO,
}


def _send(src: str, sev: int, msg: str, sock_path: str = _SOCKET_PATH) -> None:
    """Fire-and-forget datagram -- khong raise neu socket chua ton tai."""
    payload = json.dumps(
        {"t": time.time(), "src": src[:20], "sev": sev, "msg": msg},
        ensure_ascii=False,
    )
    try:
        with socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM) as s:
            s.sendto(payload.encode("utf-8", errors="replace"), sock_path)
    except OSError:
        # cc_telemetry chua khoi dong hoac /run/drone chua mount -- bo qua
        pass


class DroneLogger:
    """
    Logger nho gon gui log tu container len QGC qua cc_telemetry_node.

    Parameters
    ----------
    source : str
        Ten container/module hien thi trong QGC Messages panel.
        Vi du: "drone-mavlink", "drone-networking", "drone-vision"
    min_severity : int
        Chi gui log co severity <= gia tri nay. Mac dinh=6 (INFO tro len).
        Dat = 4 de chi gui WARNING+ (it spam hon khi production).
    sock_path : str
        Duong dan UNIX socket -- thuong khong can thay doi.
    """

    def __init__(
        self,
        source: str,
        min_severity: int = SEV_INFO,
        sock_path: str = _SOCKET_PATH,
    ) -> None:
        self._src  = source
        self._min  = min_severity
        self._sock = sock_path

    def _log(self, sev: int, msg: str) -> None:
        # So nho hon = nghiem trong hon (CRITICAL=2 < ERROR=3 < WARNING=4 < INFO=6)
        if sev <= self._min:
            _send(self._src, sev, msg, self._sock)

    def info(self, msg: str) -> None:
        """Trang trong QGC (severity=6)."""
        self._log(SEV_INFO, msg)

    def warning(self, msg: str) -> None:
        """Vang trong QGC (severity=4)."""
        self._log(SEV_WARNING, msg)

    warn = warning

    def error(self, msg: str) -> None:
        """Do trong QGC (severity=3)."""
        self._log(SEV_ERROR, msg)

    def critical(self, msg: str) -> None:
        """Do dam trong QGC (severity=2)."""
        self._log(SEV_CRITICAL, msg)


if __name__ == "__main__":
    import argparse

    p = argparse.ArgumentParser(
        description="Gui test log toi cc_telemetry qua UNIX socket"
    )
    p.add_argument("--src",  default="drone-test", help="Ten nguon (container)")
    p.add_argument("--sev",  default="info", help="Severity (critical, error, warning, info hoac 2, 3, 4, 6)")
    p.add_argument("--msg",  default="Test log from drone_log.py")
    p.add_argument("--sock", default=_SOCKET_PATH)
    args = p.parse_args()

    sev_key = str(args.sev).lower()
    sev_int = _SEV_MAP.get(sev_key, SEV_INFO)
    _send(args.src, sev_int, args.msg, args.sock)
    print(f"[drone_log] Sent: src={args.src} sev={sev_int} msg={args.msg!r} -> {args.sock}")
