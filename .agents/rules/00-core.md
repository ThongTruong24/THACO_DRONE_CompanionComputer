# Drone Edge contributor rules

## Source of truth

0. `PROJECT_CONTEXT.md` defines the active system snapshot, invariants, and fast recovery context.
0.1. `PROJECT_CONTEXT_DEEP.md` defines the historical evolution, protocol byte layouts, and deep architectural rationale.
1. `docker-compose.yml` defines the running services, profiles and runtime mounts.
2. Each `src/<module>/README.md` defines the module contract and operations.
3. Config files beside a module define its runtime behavior.
4. `README.md` defines the system topology and supported operator workflow.

Keep ports, IP ranges, service names and Makefile commands consistent across
all layers. Do not add a second deployment implementation; use `scripts/deploy.sh`.

## Host boundary

The Pi host provides SSH, Docker, boot/UART configuration and netplan. Network
interface creation is owned by `drone-networking`; do not add host networking
logic to unrelated services.

## Service boundary

- `cc-agent`: Central control engine, 2-phase configuration transactions, and Single Authority for hardware device scanning, character device validation, and serial port reservation/release (`HardwareRegistry`) via `/run/drone/hw_manager.sock` and `/run/drone/cc_agent.sock`.
- `mavlink-router-controller`: FC/SIYI UART routing, Graceful Standby Mode (UDP-only when FC is absent), and GCS delivery. Supervises `mavlink-routerd` via `/run/drone/router.sock`.
- `cc-agent`: Companion computer control plane, transaction-based configuration engine (commit/rollback), heartbeat rollback guard, and MAVLink telemetry provider (CompID 191).
- `drone-networking`: AP-STA concurrency, DHCP (`dnsmasq`), and mDNS (`thong.local`).
- `camera-stream-controller`: RealSense/V4L2 camera capture, NEON frame processing, and MediaMTX RTSP streaming (`rtsp://<pi>:8554/camera`).
- `drone-vision`: Optional AI computer vision processing (YOLO detection / landing targets); runs via `vision` profile.
- `drone-mavros`: Optional ROS 2 Jazzy bridge; connects via local UDP `127.0.0.1:14541`.
