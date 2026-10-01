# MAVROS ROS 2 & Telemetry Bridge

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](README.md)
[![ROS 2](https://img.shields.io/badge/ROS%202-Jazzy-blue.svg)](Dockerfile)
[![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi%205%20%7C%20ARM64-orange.svg)](Dockerfile)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

Cầu nối giao tiếp MAVLink sang hệ sinh thái ROS 2 Jazzy, tích hợp plugin giải mã Telemetry độc quyền `plugin_agridrone` cho máy bay nông nghiệp THACO.

---

## Mục lục (Table of Contents)
- [Tính năng chính](#tính-năng-chính)
- [Sơ đồ Kiến trúc & Luồng hoạt động](#sơ-đồ-kiến-trúc--luồng-hoạt-động)
- [Cài đặt & Biên dịch](#cài-đặt--biên-dịch)
- [Hướng dẫn Sử dụng](#hướng-dẫn-sử-dụng)
- [Đóng góp (Contributing)](#đóng-góp-contributing)
- [Giấy phép (License)](#giấy-phép-license)

---

## Tính năng chính
- **Cầu nối ROS 2 Jazzy**: Chuyển đổi hai chiều giữa các gói tin MAVLink và ROS 2 topics/services.
- **Dedicated Router Endpoint**: Kết nối tới `mavlink-router-controller` qua endpoint UDP cục bộ độc lập `127.0.0.1:14541`, tuyệt đối không can thiệp cổng FC UART hay socket proxy.
- **Agridrone Plugin**: Hỗ trợ xuất bản các topic trạng thái nông nghiệp và telemetry chuyên dụng.

---

## Sơ đồ Kiến trúc & Luồng hoạt động

### Kiến trúc Phân hệ (System Architecture)
```mermaid
graph TD
    Router[mavlink-routerd UDP :14541] <-->|UDP Endpoint| Mavros[mavros_node ROS 2]
    Mavros --> Plugin[plugin_agridrone]
    Mavros --> StateTopic[/mavros/state/]
    Mavros --> ImuTopic[/mavros/imu/data/]
    Plugin --> CC[/cc/telemetry_snapshot/]
```

### Luồng Tuần tự (Sequence Diagram)
> Mã nguồn chi tiết PlantUML: xem tại [docs/sequence.puml](docs/sequence.puml) và [docs/system.puml](docs/system.puml).

```mermaid
sequenceDiagram
    autonumber
    participant Router as mavlink-routerd (:14541)
    participant Mavros as mavros_node
    participant Plugin as plugin_agridrone
    participant ROS as ROS 2 DDS Graph

    Router->>Mavros: MAVLink Packet (UDP)
    Mavros->>Plugin: Decode Custom Packet
    Plugin->>ROS: Publish ROS 2 Message
```

---

## Cài đặt & Biên dịch (Installation & Build)

```bash
# Build Docker ARM64
make build-mavros

# Triển khai lên Raspberry Pi
make deploy-mavros
```

---

## Hướng dẫn Sử dụng (Usage)

```bash
# Khởi động container MAVROS
make start-mavros

# Xem log MAVROS
make logs-mavros

# Dừng container khi không dùng
make stop-mavros
```

---

## Đóng góp (Contributing)
Mọi đề xuất đóng góp vui lòng tuân thủ quy tắc quản trị tài liệu tại [`.agents/rules/06-doc-sync-and-rule-governance.md`](../../.agents/rules/06-doc-sync-and-rule-governance.md).

---

## Giấy phép (License)
Dự án được phân phối dưới giấy phép [MIT License](https://opensource.org/licenses/MIT).

## GitHub / GHCR deployment contract

Source/config templates are pulled from GitHub; images are pulled from GHCR.
Set the actual lowercase `IMAGE_NAMESPACE` in private `.env`; choose a commit
SHA or development `IMAGE_TAG`. All internal runtime images target linux/arm64;
Pi Compose has no build definitions. See [deployment guide](../../deploy/README.md).

WSL: `make publish-mavros` after commit/push. Pi: `bash deploy/update.sh --service mavros`.

Compose service: `mavros`; GHCR image: `drone-mavros`; build context: `src/mavros`.
ROS Jazzy workspace/plugins are built inside the ARM64 image, never on Pi.
