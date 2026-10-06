# Drone Edge Computing - Deployment & Architecture Suite (`deploy/`)

Thư mục `deploy/` được quy hoạch tinh gọn theo **5 nhóm chức năng chuyên biệt**, tách bạch hoàn toàn giữa **Build** và **Ship/Deploy**, kết hợp cơ chế phân giải mục tiêu thông minh (Zero-Conf mDNS ➔ Fallback IP).

---

## 📁 Cấu trúc Thư mục

```text
deploy/
├── README.md                      # Tài liệu tổng quan kiến trúc triển khai
├── deploy.sh                      # Lệnh nhanh gọi deploy/ship/deploy.sh
├── setup/                         # Khởi tạo bo mạch và môi trường ban đầu
│   ├── setup.sh                   # Wizard thông minh cài SSH + Docker + Hardware
│   ├── setup-ssh.sh               # Shortcut cấu hình SSH Key
│   ├── setup_hardware.sh          # Shortcut cấu hình phần cứng
│   ├── install_docker_wsl.sh      # Setup Docker trên môi trường dev WSL
│   └── hardware/                  # [HAL] Cấu hình phần cứng theo từng bo mạch
│       ├── jetson/hardware.env    # NVIDIA Jetson (Orin / NX)
│       ├── rpi4/                  # Raspberry Pi 4B (H.264 v4l2m2m HW block)
│       │   ├── config.txt
│       │   └── hardware.env
│       └── rpi5/                  # Raspberry Pi 5 (RP1 uart0-pi5 + Cortex-A76 NEON)
│           ├── config.txt
│           └── hardware.env
├── build/                         # Biên dịch Docker ARM64 & Đóng gói Release
│   ├── README.md                  # 📖 Hướng dẫn chi tiết: Cách build chéo ARM64 & đóng gói
│   ├── cross_build.sh             # Thiết lập Docker Buildx ARM64 từ host x86
│   └── package_release.sh         # Đóng gói release tarball offline
├── ship/                          # Phân giải Target mDNS/IP, Đẩy code & Triển khai từ xa
│   ├── README.md                  # 📖 Hướng dẫn chi tiết: Phân giải IP thông minh & vận hành
│   ├── target.env                 # Cấu hình mDNS domain, Fallback IP và SSH user
│   ├── target.example.env         # Bản mẫu cấu hình target
│   ├── resolve_target.sh          # Bộ phân giải mục tiêu 2 nấc (mDNS ➔ Fallback IP)
│   ├── deploy.sh                  # Script chính đồng bộ mã nguồn & khởi chạy container
│   └── verify-deployment.sh       # Xác thực sức khỏe container & sockets sau khi deploy
├── monitor/                       # Công cụ chẩn đoán, giám sát & logs
│   ├── doctor.sh                  # Bác sĩ hệ thống quét tuần tự toàn bộ kết nối & phần cứng
│   ├── perf.sh                    # Giám sát CPU, RAM, nhiệt độ & throttling
│   └── mavlink-status.sh          # Giám sát cổng & lưu lượng gói tin MAVLink Router
└── systemd/                       # File service cho hệ điều hành Host
    └── drone-edge.service         # Service Systemd Autostart duy nhất (kích hoạt Docker)
```

---

## 🎯 1. Phân nhóm Chức năng Chi tiết

### 1.1. `deploy/setup/` — Khởi tạo Bo mạch, Môi trường & Phần cứng (HAL)
- **`setup.sh`**: Setup Wizard thông minh, tự động hỏi có cài SSH Key không, có cài Docker không, và nhận diện bo mạch để cấu hình phần cứng.
- **`setup/hardware/`**:
  - `rpi5/`: Bắt buộc nạp overlay `uart0-pi5` cho chip RP1.
  - `rpi4/`: Bật `enable_uart=1` cho SoC BCM2711.
  - `jetson/`: Cổng UART `/dev/ttyTHS1`, `/dev/ttyTHS2`.
  - `hardware.env` chỉ chứa cổng/baud UART (`DRONE_*`, `SIYI_*`); cấu hình encoder và stream nằm trong `src/camera-stream-controller/camera.yaml`, vision trong `src/drone-vision/drone.yaml`.

### 1.2. `deploy/build/` — Biên dịch Docker ARM64 & Đóng gói Release
- **Xem chi tiết tại: [deploy/build/README.md](build/README.md)**.
- **`cross_build.sh`**: Biên dịch chéo đa kiến trúc (ARM64) cho Raspberry Pi / Jetson từ máy dev x86 qua Docker Buildx.
- **`package_release.sh`**: Đóng gói toàn bộ source và cấu hình thành file `tar.gz` độc lập để triển khai ngoài thực địa không cần internet.

### 1.3. `deploy/ship/` — Phân giải Mục tiêu Thông minh & Triển khai Từ xa
- **Xem chi tiết tại: [deploy/ship/README.md](ship/README.md)**.
- **`resolve_target.sh`**: Bộ phân giải mục tiêu 2 nấc:
  1. Kiểm tra mDNS `drone.local` qua cổng SSH port 22 trong 1.5s. Nếu phản hồi, tự động dùng `drone.local` (không cần gõ IP).
  2. Nếu mDNS rớt, tự động chuyển về `TARGET_FALLBACK_IP` khai báo trong `target.env`.
- **`deploy.sh`**: Đồng bộ mã nguồn, cấu hình và khởi động container từ xa. Hỗ trợ deploy toàn bộ hoặc từng container đơn lẻ (`make deploy-cc-agent`).
- **`verify-deployment.sh`**: Kiểm tra container status và sockets ngay sau deploy.

### 1.4. `deploy/monitor/` — Chẩn đoán, Giám sát & Logs
- **`doctor.sh`**: Bộ kiểm tra tuần tự 8 chặng: Serial FCU & SIYI, RealSense camera, MAVLink router, luồng telemetry heartbeat, RTSP video streaming, WiFi AP & mDNS.
- **`perf.sh`**: Thống kê tải CPU/RAM, nhiệt độ, FPS video và phát hiện sụt áp (throttled).
- **`mavlink-status.sh`**: Thống kê gói tin MAVLink TX/RX trên từng endpoint.

### 1.5. `deploy/systemd/` — Dịch vụ Autostart Host OS
- Chứa `drone-edge.service`: File cấu hình Systemd duy nhất trên Host OS.
- Tự động chạy `docker compose up -d` khi cấp nguồn và tắt an toàn khi shutdown.
- Cài đặt qua lệnh: `make autostart`.

---

## 📌 2. Vị trí Cấu hình WiFi (`wifi.json`)

- Cấu hình WiFi (`config/wifi.json`, không được commit) và bản mẫu `config/wifi.example.json` nằm tại **`config/`** ở gốc repo. Đây là trạng thái riêng của từng drone: `deploy.sh` chỉ seed lên Pi một lần và không bao giờ ghi đè.
- Container `drone-networking` mount thư mục này ở chế độ chỉ đọc:
  ```yaml
  volumes:
    - ./config:/app/config:ro
  ```

---

## 🚀 3. Cheat Sheet Lệnh Vận hành (Zero-Conf)

Nhờ bộ phân giải `resolve_target.sh`, **bạn không cần truyền `PI_HOST=...`**:

| Thao tác | Lệnh thực hiện |
| :--- | :--- |
| **Deploy toàn bộ stack** | `make deploy` (Tự động nhận `drone.local` hoặc fallback IP) |
| **Deploy riêng 1 service** | `make deploy-cc-agent`<br>`make deploy-networking`<br>`make deploy-mavlink` |
| **Deploy không build lại** | `make deploy-no-build` |
| **Chẩn đoán sức khỏe hệ thống** | `make doctor` |
| **Kiểm tra trạng thái container** | `make ps` |
| **Xem thống kê lưu lượng MAVLink**| `make logs-mavlink-stats` |
| **Kiểm tra nhiệt độ / CPU / RAM** | `make perf` |
| **Xem log realtime toàn bộ** | `make logs-all` |
| **Deploy chỉ định IP thủ công** | `make deploy PI_HOST=10.14.95.6` |
