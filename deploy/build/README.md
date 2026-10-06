# Hướng dẫn Build Docker ARM64 & Đóng gói Release (`deploy/build/`)

Thư mục `deploy/build/` chứa toàn bộ công cụ tự động hóa quy trình:

1. **Biên dịch chéo mã nguồn & đóng gói Docker Images ARM64** (`cross_build.sh` qua Docker Buildx).

> 💡 **Lưu ý**: Để triển khai mã nguồn và container lên Drone từ xa, xem tài liệu chi tiết tại **[`deploy/ship/README.md`](../ship/README.md)**.

---

## 📂 Danh sách các Script trong `deploy/build/`

| File | Vai trò |
| :--- | :--- |
| **[`cross_build.sh`](cross_build.sh)** | Cấu hình Docker Buildx đa nền tảng (`linux/arm64`) trên máy tính phát triển x86. |

## 🛠️ 1. BUILD NHƯ THẾ NÀO? (How to Build)

Toàn bộ hệ thống chạy trên nền tảng vi xử lý **ARM64** (Raspberry Pi 5 Cortex-A76 / Pi 4B Cortex-A72 / Jetson). Bạn có thể build theo 2 cách:

### Cách 1: Build chéo (Cross-build) từ máy PC/Laptop (Khuyên dùng - Nhanh nhất)

Máy tính dev (x86_64) dùng Docker Buildx để biên dịch ảnh ARM64 trong vài phút thay vì hàng giờ trên Pi:

```bash
# 1. Khởi tạo môi trường Docker Buildx đa nền tảng (chỉ cần chạy 1 lần trên máy dev):
./deploy/build/cross_build.sh

# 2. Build chéo toàn bộ các Docker images sang định dạng ARM64:
make build-all
```

### Cách 2: Build trực tiếp trên Raspberry Pi / Jetson

Nếu bạn đang ngồi trực tiếp trên terminal của bo mạch:

```bash
cd ~/drone-edge
docker compose build
```

### Bảng tra cứu Lệnh Build Từng Module

| Module | Lệnh Build | Mô tả |
| :--- | :--- | :--- |
| **All Core** | `make build-all` | Build cả 5 core images (hw, mavlink, agent, net, cam) |
| **Hardware** | `make build-hw` | C++ Hardware Manager (`drone_hardware_manager`) |
| **MAVLink** | `make build-mavlink` | MAVLink Router + C++ Supervisor |
| **Agent** | `make build-cc-agent` | C++ CC Control Plane Agent |
| **Networking** | `make build-networking` | Hostapd AP + Dnsmasq DHCP + mDNS |
| **Camera** | `make build-camera-rtsp` | C++ NEON Frame Grabber + MediaMTX RTSP |
| **Vision** | `make build-vision` | YOLOv8 Tensor/ONNX AI Object Detection |
| **MAVROS** | `make build-mavros` | ROS 2 Jazzy + Telemetry Dialect Plugin |

---
