# MAVLink Router (Data Plane Supervisor)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](README.md)
[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](CMakeLists.txt)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20ARM64%20%7C%20x86__64-orange.svg)](Dockerfile)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

Dịch vụ giám sát và điều phối luồng MAVLink tập trung cho Drone Edge, bọc ngoài tiến trình `mavlink-routerd`. Đảm bảo hệ thống hoạt động ổn định nhờ chế độ **Graceful Standby Mode**, độc lập với DDS khi khởi động, hỗ trợ cấu hình động qua ROS 2 Service `/cc/router/apply_links` và Topic `/cc/link_config`.

---

## Mục lục
- [Tính năng nổi bật](#tính-năng-nổi-bật)
- [Sơ đồ Kiến trúc & Luồng hoạt động](#sơ-đồ-kiến-trúc--luồng-hoạt-động)
- [Cài đặt & Biên dịch](#cài-đặt--biên-dịch)
- [Cấu hình & Biến môi trường](#cấu-hình--biến-môi-trường)
- [Giao diện ROS 2 & Lệnh điều khiển](#giao-diện-ros-2--lệnh-điều-khiển)
- [Kiểm thử Tự động](#kiểm-thử-tự-động)

---

## Tính năng nổi bật
- **Độc lập với DDS khi khởi động**: `MavlinkRouter` đọc cấu hình từ file active (`/run/drone/links.json`) hoặc default (`/app/config/links.json`) và khởi chạy `mavlink-routerd` **trước** `rclcpp::init()`. Nếu ROS 2 gặp sự cố, luồng routerd vẫn tiếp tục hoạt động thông suốt.
- **Graceful Standby Mode**: Tự động kiểm tra tính khả dụng của từng cổng serial bằng `stat()` + `S_ISCHR`. Nếu cổng tạm thời vắng mặt, endpoint tương ứng được cấu hình ở chế độ standby mà không làm crash routerd.
- **Tái cấu hình động không downtime**: Nhận yêu cầu cấu hình mới qua ROS 2 Service `/cc/router/apply_links` hoặc subscriber `/cc/link_config`, tự động sinh lại file cấu hình và nạp lại routerd.
- **Một đường restart duy nhất, có khóa**: Mọi lần khởi động lại `mavlink-routerd` (watchdog khi child thoát, hotplug kiểm tra cổng định kỳ mỗi 5 s, áp dụng cấu hình mới) đều đi qua cùng một hàm giữ `config_mutex_`. Dừng tiến trình con một cách an toàn bằng `SIGTERM`, chờ thoát rồi mới `SIGKILL`.
- **SerialLinkMonitor**: Giám sát lưu lượng MAVLink kết hợp bộ đếm lỗi UART của kernel. Áp dụng cơ chế giữ trạng thái `CONNECTED` thêm 3 tick sau lần online cuối để chống chập chờn, tự động phát hiện chuyển trạng thái và gửi thông báo qua `/cc/mavlink_log`.

---

## Sơ đồ Kiến trúc & Luồng hoạt động

### Kiến trúc Phân hệ (System Architecture)
```mermaid
graph TD
    UART[Generic Serial Links] --> RouterD[mavlink-routerd Core]
    RouterD --> QGC[QGC GCS UDP :14550]
    RouterD --> CCAgent[Config Agent UDP :14600]
    
    Supervisor[MavlinkRouter C++] -->|Manages Child PID| RouterD
    Node[MavlinkRouterNode ROS 2] -->|Controls| Supervisor
    Node -->|Service /cc/router/apply_links| Srv[ApplyLinks Service]
    Node -->|Publishes| Status[/cc/serial_link_status]
```

### Luồng Hoạt động (Sequence Diagram)
Chi tiết xem tại [docs/sequence.puml](docs/sequence.puml) và [docs/system.puml](docs/system.puml).

---

## Cài đặt & Biên dịch

### Biên dịch với colcon
```bash
colcon build --packages-select mavlink_router
```

---

## Cấu hình & Biến môi trường

| Biến môi trường | Mặc định | Ý nghĩa |
|---|---|---|
| `ROUTER_LINKS_ACTIVE` | `/run/drone/links.json` | Đường dẫn file cấu hình links đang hoạt động (tmpfs) |
| `ROUTER_LINKS_DEFAULT` | `/app/config/links.json` | Đường dẫn file cấu hình links mặc định bền vững |
| `GCS_IP` | *(rỗng)* | Địa chỉ IP unicast tĩnh của máy trạm GCS (chỉ nhận unicast) |
| `TCP_SERVER_PORT` | `5760` | Cổng TCP server proxy MAVLink |

---

## Giao diện ROS 2 & Lệnh điều khiển

- **Service**: `/cc/router/apply_links` (`cc_msgs/srv/ApplyLinks`)
- **Subscription**: `/cc/link_config` (`cc_msgs/msg/LinkConfig`, TransientLocal QoS)
- **Publisher**: `/cc/serial_link_status` (`cc_msgs/msg/SerialLinkStatus`, 1 Hz)
- **Publisher**: `/cc/mavlink_log` (`cc_msgs/msg/MavlinkLog`)

---

## Kiểm thử Tự động
```bash
colcon test --packages-select mavlink_router && colcon test-result --verbose
```
