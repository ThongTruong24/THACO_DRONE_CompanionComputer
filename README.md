# Drone Edge Companion Computer

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](README.md)
[![Tests Passing](https://img.shields.io/badge/tests-33%2F33%20passing-brightgreen.svg)](tests/)
[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](CMakeLists.txt)
[![Platform](https://img.shields.io/badge/target-Raspberry%20Pi%205%20%7C%20ARM64-orange.svg)](docker-compose.yml)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

Hệ sinh thái Companion Computer phân tán trên nền tảng Raspberry Pi 5 cho máy bay nông nghiệp AgriDrone (THACO). Toàn bộ hệ thống được xây dựng trên chuẩn ngôn ngữ **C++20 Native** kết hợp kiến trúc micro-container Docker độc lập, tối ưu hóa giao tiếp nội bộ qua Unix Domain Sockets và mạng UDP độ trễ thấp.

---

## Mục lục (Table of Contents)
- [Tổng quan Phân hệ](#tổng-quan-phân-hệ)
- [Sơ đồ Kiến trúc Tổng thể](#sơ-đồ-kiến-trúc-tổng-thể)
- [Yêu cầu Hệ thống](#1-yêu-cầu-hệ-thống)
- [Cấu hình Lần đầu](#2-cấu-hình-lần-đầu)
- [Kiểm thử Tập trung CTest](#3-kiểm-thử-tập-trung-c-unit--integration-tests)
- [Biên dịch Docker ARM64](#4-cách-build-docker-images-arm64-trên-wsl2)
- [Triển khai Lên Raspberry Pi](#5-triển-khai-deploy-lên-raspberry-pi)
- [Vận hành & Giám sát Hệ thống](#6-vận-hành--giám-sát-hệ-thống)
- [Quy tắc Dự án & Quản trị Quy chuẩn](#7-quy-tắc-dự-án--quản-trị-hệ-thống)
- [Đóng góp (Contributing)](#8-đóng-góp-contributing)
- [Giấy phép (License)](#9-giấy-phép-license)

---

## Tổng quan Phân hệ

| Phân hệ / Module | Đường dẫn mã nguồn | Vai trò cốt lõi | Hồ sơ Kỹ thuật |
|---|---|---|:---:|
| **Companion Agent & Hardware Registry** | [`src/cc-agent`](src/cc-agent/) | Bộ não điều khiển, quản lý độc quyền phần cứng (`HardwareRegistry` qua `/run/drone/hw_manager.sock`), 2-phase config transactions, 30s rollback guard | [Docs & UML](src/cc-agent/docs/) |
| **MAVLink Router** | [`src/mavlink-router-controller`](src/mavlink-router-controller/) | Điều phối MAVLink, **Graceful Standby Mode**, router UDP `:14550`, `:14541`, `:14600` | [Docs & UML](src/mavlink-router-controller/docs/) |
| **Networking** | [`src/drone-networking`](src/drone-networking/) | Phát WiFi AP (`AP_DRONE`), cấp DHCP `dnsmasq`, mDNS `thong.local` | [Docs & UML](src/drone-networking/docs/) |
| **Camera RTSP** | [`src/camera-stream-controller`](src/camera-stream-controller/) | Thu hình RealSense/V4L2, tăng tốc ARM NEON, MediaMTX RTSP `/camera` | [Docs & UML](src/camera-stream-controller/docs/) |
| **AI Vision** | [`src/drone-vision`](src/drone-vision/) | Suy luận YOLOv8, phát hiện vật thể, đẩy luồng annotated `/yolo` | [Docs & UML](src/drone-vision/docs/) |
| **MAVROS** | [`src/mavros`](src/mavros/) | Cầu nối ROS 2 Jazzy, giải mã custom telemetry plugin | [Docs & UML](src/mavros/docs/) |

---

## Sơ đồ Kiến trúc Tổng thể

```mermaid
graph TD
    subgraph "External Entities"
        FC[Flight Controller<br/>Cube Orange Plus]
        GCS[QGroundControl GCS<br/>UDP :14550]
        Web[Web Browser<br/>WebRTC :8889]
        VLC[Video Player / QGC<br/>RTSP :8554]
    end

    subgraph "Raspberry Pi 5 Companion Computer"
        HWMgr[hardware-manager<br/>/run/drone/hw_manager.sock]
        Router[mavlink-router-controller<br/>/run/drone/router.sock]
        CCAgent[cc-agent<br/>Control Plane UDP :14600]
        Cam[camera-stream-controller<br/>NEON + MediaMTX]
        Net[drone-networking<br/>AP-STA + DHCP]
        Vision[drone-vision<br/>YOLOv8 Pipeline]
        Mavros[drone-mavros<br/>ROS 2 Jazzy Bridge]
    end

    FC <-->|UART /dev/ttyAMA4| Router
    Router <-->|UDP :14550| GCS
    Router <-->|UDP :14600| CCAgent
    Router <-->|UDP :14541| Mavros
    Router -.->|Validate Port| HWMgr
    CCAgent -.->|Reload / Status| Router
    Cam -->|RTSP :8554/camera| VLC
    Cam -->|WebRTC :8889/camera| Web
    Cam -->|Ingest Stream| Vision
    Vision -->|RTSP :8554/yolo| VLC
```

---

## 1. Yêu cầu Hệ thống

### Máy phát triển / Build (WSL2 Ubuntu 24.04)
- Docker Engine + Compose + Buildx; user có quyền Docker. Builder `drone-builder` hỗ trợ ARM64 qua QEMU khi cần.
- C++20 toolchain (`g++-13` hoặc mới hơn, `cmake >= 3.28`, `libgtest-dev`, `libyaml-cpp-dev`, `libgstreamer1.0-dev`).
- SSH client, `make`, `bash`.
- Kết nối mDNS tới Pi: `thong.local` hoặc IP tĩnh qua LAN/WiFi.

### Raspberry Pi 5
- Raspberry Pi OS / Ubuntu 64-bit (aarch64), Docker Engine + Docker Compose plugin.
- User `thong` thuộc group `docker`.
- UART4 đã được bật cho Flight Controller (`/dev/ttyAMA4` ở 921600 baud); UART0 cho SIYI Air Unit (`/dev/ttyAMA0` ở 115200 baud).
- Cổng USB 3.0 màu xanh cho camera RealSense.

---

## 2. Cấu hình Lần đầu

Tại thư mục repository trong WSL:

```bash
cp .env.example .env
```

Kiểm tra nội dung `.env`:

```dotenv
PI_HOST=thong.local
PI_USER=thong
MDNS_HOSTNAME=thong
DRONE_SERIAL_PORT=/dev/ttyAMA4
DRONE_BAUD_RATE=921600
GCS_IP=
```

Thiết lập SSH key không mật khẩu tới Pi:

```bash
make setup-ssh
ssh drone-pi
```

Nếu Pi mới thiết lập lần đầu (cài Docker và nạp cấu hình UART4 vào `config.txt`):

```bash
make setup-pi
# Sau khi Pi reboot, kích hoạt tự khởi động khi cấp nguồn:
make autostart
```

---

## 3. Kiểm thử Tập trung (C++ Unit & Integration Tests)

Dự án áp dụng tiêu chuẩn kiểm thử **100% C++ GoogleTest & CTest** cho cả 4 module lõi và 2 luồng tích hợp toàn hệ thống:

```bash
# Biên dịch toàn bộ mã nguồn C++ và bài test
make build-native

# Chạy toàn bộ 33 bài test tự động qua CTest
make test
```

### Bảng phân bố kiểm thử (30/30 Tests PASSED):
| Module | File Test | Số lượng | Nội dung kiểm thử |
|---|---|:---:|---|
| `cc-agent` | `src/cc-agent/tests/` | 16 | Transaction engine commit/rollback, Heartbeat rollback guard, đọc metrics hệ thống, HardwareRegistry (Scan, Health, Validate, Reserve/Release) |
| `mavlink-router-controller` | `src/mavlink-router-controller/tests/` | 6 | Sinh config Standby & Active mode, IPC reload, giám sát tiến trình con |
| `camera-stream-controller` | `src/camera-stream-controller/tests/` | 3 | Parse cấu hình YAML camera, fallback stub camera, xử lý khung hình NEON |
| **Root Integration (IPC)** | `tests/test_ipc_hardware_router.cpp` | 3 | Giao tiếp UDS thực giữa UdsHubServer (HardwareSubsystemHandler) và RouterSupervisor |
| **Root Integration (MAVLink)** | `tests/test_mavlink_loopback.cpp` | 2 | Loopback UDP thực: MAVLink Heartbeat và Custom Telemetry Dialect THACO (ID 42014) |

---

## 4. Cách Build Docker Images ARM64 (trên WSL2)

GitHub lưu source/config; GHCR lưu images. Pi chỉ pull và chạy container.
Thiết lập private `.env` từ `.env.example`: `REGISTRY=ghcr.io`, điền
`IMAGE_NAMESPACE` là owner/org thực tế, chọn `IMAGE_TAG=dev-ai-tracking-flow`.
Không lưu registry token trong repository hoặc `.env`.

```bash
# Sau khi review, commit và git push:
make publish-core IMAGE_TAG=dev-ai-tracking-flow
# Optional, cùng Git commit với core dependencies:
make publish-mavros IMAGE_TAG=dev-ai-tracking-flow
make publish-vision IMAGE_TAG=dev-ai-tracking-flow
```

Buildx dùng `drone-builder`, `linux/arm64`, `--push`. Mỗi publication có full Git
SHA tag và development tag; kiểm tra manifest/platform/revision sau push.
`make build-core` / `make build-all` vẫn hỗ trợ build local `--load` để debug.
Vision publish tự build/push base GHCR trước app; không phụ thuộc local base image.

```mermaid
flowchart LR
    W[WSL x86_64] -->|git push| G[GitHub source/config]
    W -->|Buildx ARM64 --push| R[GHCR images]
    G -->|git pull| P[Pi ARM64]
    R -->|compose pull| P
    P -->|up --no-build| C[Runtime containers]
```

## 5. Triển khai (Deploy) Lên Raspberry Pi

Đọc [deploy/README.md](deploy/README.md) để provisioning config riêng, đăng nhập
GHCR, migration `.env`/WiFi cũ và hướng dẫn source/image revision matching.
Clone Git repository trên Pi; không compile source. `.env`, WiFi credentials và
runtime state phải được giữ ngoài Git. Camera/Vision defaults, scripts, logging
helpers và model `yolo26n.pt` được Git pull; có thể chọn private config overrides.

```bash
# Chạy TRÊN Pi, sau khi provision config riêng:
cd ~/drone-edge
make update
# Optional:
bash deploy/update.sh --service mavros
bash deploy/update.sh --service drone-vision
```

`make update` chạy Git pull fast-forward, Compose pull, kiểm tra ARM64/revision,
up với `--no-build --pull never` rồi verify. Named volumes được giữ lại.
Không apt install, netplan, UART setup, boot edit hoặc reboot trong update.

```bash
# Manual equivalent, với .env và runtime config đã có:
git pull --ff-only
docker compose pull
docker compose up -d --no-build --pull never
docker compose ps
```

`make deploy` / `make deploy-fast` là tiện ích tùy chọn qua SSH để chạy cùng flow
Git/registry trên Pi. Archive/rsync chỉ còn ở `make deploy-offline` (legacy).
Tailscale là profile **MANUAL / UNRESOLVED**, không thuộc core publication/update.

---

## 6. Vận hành & Giám sát Hệ thống

### Điều khiển từ máy phát triển (WSL2):
```bash
# Xem trạng thái các container đang chạy trên Pi
make ps

# Xem log thời gian thực:
make logs-mavlink        # Log MAVLink Router
make logs-cc-agent       # Log Companion Agent
make logs-networking     # Log WiFi & DHCP
make logs-camera-rtsp    # Log Camera RealSense & MediaMTX
make logs-vision         # Log YOLO Vision

# Khởi động lại từng service khi cần:
make restart-mavlink
make restart-camera-rtsp

# Chẩn đoán tự động toàn diện:
make doctor
```

### Endpoints mạng và Truy cập:
| Dịch vụ | Giao thức / Cổng | Địa chỉ kết nối |
|---|---|---|
| **WiFi Hotspot** | 802.11 AP | SSID: `AP_DRONE` (Mật khẩu: `12345678`) |
| **AP Gateway Recovery** | IPv4 Static | `192.168.10.1` |
| **MAVLink QGC** | UDP Bi-directional | `thong.local:14550` (hoặc `192.168.10.1:14550`) |
| **MAVLink TCP Server** | TCP | `thong.local:5760` |
| **MAVLink MAVROS Local**| UDP Local | `127.0.0.1:14541` (Local socket) |
| **Camera RTSP Raw** | RTSP / TCP | `rtsp://thong.local:8554/camera` |
| **Camera WebRTC** | HTTP H5 | `http://thong.local:8889/camera` |
| **YOLO Video Stream** | RTSP / TCP | `rtsp://thong.local:8554/yolo` |

---

## 7. Quy tắc Dự án & Quản trị Hệ thống

Tất cả các quy tắc phát triển, ranh giới dịch vụ và quy trình bảo vệ cấu hình được quản lý tại thư mục [`.agents/rules/`](.agents/rules/):
- [`00-core.md`](.agents/rules/00-core.md): Nguồn chân lý, ranh giới phần cứng và định nghĩa 7 phân hệ.
- [`01-networking.md`](.agents/rules/01-networking.md): Quy định AP-STA, DHCP và Netplan.
- [`02-mavlink.md`](.agents/rules/02-mavlink.md): Single Source of Truth cho MAVLink packets (`thaco_common.xml`), Standby Mode và cấm broadcast 14550.
- [`03-camera-rtsp.md`](.agents/rules/03-camera-rtsp.md): Ranh giới camera stream và MediaMTX.
- [`04-vision.md`](.agents/rules/04-vision.md): Quy định pipeline AI YOLO và an toàn bay.
- [`05-doctor.md`](.agents/rules/05-doctor.md): Ma trận mã lỗi chẩn đoán tự động tuần tự.
- [`06-doc-sync-and-rule-governance.md`](.agents/rules/06-doc-sync-and-rule-governance.md): Quy định bắt buộc đồng bộ tài liệu, bắt buộc hồ sơ UML (`sequence.puml`, `system.puml`) cho mỗi module, và chốt chặn an toàn: **Hỏi ý kiến người dùng trước khi sửa bất kỳ rule nào**.

---

## 8. Đóng góp (Contributing)
1. Fork repository và tạo nhánh mới: `git checkout -b feature/amazing-feature`.
2. Tuân thủ tiêu chuẩn lập trình **C++20**.
3. Chạy toàn bộ test suite để đảm bảo không lỗi hồi quy: `make test`.
4. Cập nhật đầy đủ `README.md` và các sơ đồ UML trong thư mục `docs/` theo quy định tại `Rule 06`.
5. Tạo Pull Request với mô tả chi tiết.

---

## 9. Giấy phép (License)
Dự án được phân phối dưới giấy phép mã nguồn mở [MIT License](https://opensource.org/licenses/MIT).
