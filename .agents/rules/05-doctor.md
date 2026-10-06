# 05 — QUY TRÌNH CHẨN ĐOÁN MÃ LỖI TUẦN TỰ (scripts/doctor.sh)

Lệnh thực thi: `make doctor` (hoặc `make check`).

## MA TRẬN MÃ LỖI (ERROR CODE MATRIX)
| Bước kiểm tra | Mã lỗi trả về | Nguyên nhân | Khắc phục nhanh |
|---|---|---|---|
| **1. Kết nối Pi** | `ERR_00_CANNOT_CONNECT_PI` | Mất nguồn hoặc sai IP | Kiểm tra cáp LAN / nguồn Pi. |
| **2. Cổng Serial** | `ERR_01_UART4_MISSING` | Thiếu `/dev/ttyAMA4` | Kiểm tra `config/hardware/` hoặc chạy `setup_hardware.sh`, reboot Pi (Router chạy UDP Standby mode). |
| **2. Camera** | `ERR_04_REALSENSE_NOT_USB3` | Cắm nhầm USB 2.0 | Đổi sang cổng USB 3.0 màu xanh. |
| **3. MAVLink** | `ERR_20_MAVLINK_CONTAINER_DOWN` | Container `mavlink-router-controller` chưa chạy | Chạy `make start-mavlink`. |
| **3. Mở UART4** | `ERR_21_MAVLINK_UART4_OPEN_FAIL` | Không mở được UART4 | Kiểm tra tiến trình chiếm cổng / baudrate trong `.env`. |
| **4. Heartbeat** | `ERR_24_NO_HEARTBEAT` | Mở cổng OK nhưng không nhận data trong 3s | 1. Đảo ngược dây TX/RX (GPIO 8 nối RX FC, GPIO 9 nối TX FC).<br>2. Cấu hình ArduPilot/PX4: `SERIALx_PROTOCOL=2`, `SERIALx_BAUD=115`. |
| **5. Camera + RTSP** | `ERR_30_CAMERA_RTSP_DOWN` | Container `camera-stream-controller` chưa chạy | Chạy `make start-camera-rtsp`. |
| **5. Cổng RTSP 8554** | `ERR_31_RTSP_PORT_BLOCKED` | Cổng 8554 chưa mở | Kiểm tra log MediaMTX: `make logs-camera-rtsp`. |
| **6. Networking** | `ERR_11_HOSTAPD_FAIL` | Container `drone-networking` dừng | Chạy `make start-networking`. |
| **6. DHCP Server** | `ERR_12_DNSMASQ_FAIL` | Dnsmasq không chạy | Kiểm tra `src/drone-networking/dnsmasq.conf`. |
