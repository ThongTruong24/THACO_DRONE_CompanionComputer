# PROJECT CONTEXT SNAPSHOT (FAST RECOVERY)

> **MỤC ĐÍCH**: Đây là file ngữ cảnh Tầng 1 (Fast Snapshot) tóm tắt toàn bộ trạng thái kiến trúc, cổng giao tiếp, quy chuẩn kiểm thử và quyết định thiết kế cốt lõi của repository `Drone`. AI Assistant BẮT BUỘC đọc file này ngay khi bắt đầu phiên mới hoặc sau khi context bị nén (compaction).

---

## 1. BẤT BIẾN KIẾN TRÚC CỐT LÕI (CORE INVARIANTS)
1. **100% C++ Standard**: Toàn bộ các module lõi điều khiển (`hardware-manager`, `mavlink-router-controller`, `cc-agent`, `camera-stream-controller`) được viết bằng **C++20 Native**, biên dịch bằng `cmake` và kiểm thử tự động bằng GoogleTest/CTest (33/33 tests passing).
2. **Single Hardware Authority**: `cc-agent` (thông qua `HardwareRegistry` và socket `/run/drone/hw_manager.sock`) là cơ quan DUY NHẤT có thẩm quyền quét thiết bị ký tự (`/dev/tty*`), xác thực cổng và cấp phát cổng serial độc quyền. Module `hardware-manager` cũ đã được sáp nhập trực tiếp vào `cc-agent` để giảm thiểu overhead IPC và tối ưu hóa tài nguyên.
3. **Graceful Standby Mode**: `mavlink-router-controller` tuyệt đối không được crash khi thiếu Flight Controller (UART `/dev/ttyAMA4`). Nếu FC vắng mặt, router tự động khởi động ở chế độ UDP Standby mở sẵn các cổng `:14550`, `:14600`, `:14541`.
4. **Single Source of Truth MAVLink**: Mọi định nghĩa gói tin MAVLink custom CHỈ ĐƯỢC PHÉP chỉnh sửa tại file `$HOME/Mavlink/custom/thaco_common.xml`, sau đó chạy `$HOME/Mavlink/sync_all.sh`. Tuyệt đối không sửa tay header generated.
5. **Cấm Broadcast 14550**: Do router chạy `network_mode: host`, cấm cấu hình endpoint broadcast (`192.168.10.255`, `LAN_wlan0`) trên cổng 14550 để chống vòng lặp Telemetry Amplification.

---

## 2. BẢN ĐỒ 6 PHÂN HỆ (MODULE TOPOLOGY)

| Phân hệ (Module) | Thư mục mã nguồn | Container Service | Cổng / Socket giao tiếp | Chức năng chính |
|---|---|---|---|---|
| **Companion Agent & HW Registry** | `src/cc-agent` | `cc-agent` | `/run/drone/hw_manager.sock`<br>`/run/drone/cc_agent.sock`<br>UDP `:14600` (CompID 191) | Quản lý độc quyền phần cứng (HardwareRegistry), Transaction 2-pha, rollback guard 30s |
| **MAVLink Router** | `src/mavlink-router-controller` | `mavlink-router-controller` | `/run/drone/router.sock`<br>UDP `:14550`, `:14541`, `:14600` | Điều phối MAVLink, standby mode, reload IPC |
| **Networking** | `src/drone-networking` | `drone-networking` | AP `uap0` (192.168.10.1)<br>DNS 53, mDNS 5353 (`thong.local`) | AP-STA Concurrency, DHCP `dnsmasq`, mDNS |
| **Camera RTSP** | `src/camera-stream-controller` | `camera-stream-controller` | RTSP `:8554/camera`<br>WebRTC `:8889/camera` | Thu hình RealSense/V4L2, tăng tốc NEON SIMD |
| **AI Vision** | `src/drone-vision` | `drone-vision` (profile `vision`) | RTSP `:8554/yolo` | Suy luận YOLOv8, phát hiện vật thể |
| **MAVROS** | `src/mavros` | `mavros` (image `drone-mavros`, profile `mavros`) | MAVLink UDP `:14541` (Local socket) | Cầu nối ROS 2 Jazzy, telemetry plugin |

---

## 3. BẢNG CỔNG MẠNG & PHẦN CỨNG (ENDPOINTS & HARDWARE)
- **UART4 (`/dev/ttyAMA4`)**: Flight Controller (Cube Orange Plus) @ `921600` baud.
- **UART0 (`/dev/ttyAMA0`)**: SIYI Air Unit Telemetry @ `115200` baud.
- **Ethernet (`eth0`)**: Mạng dây tĩnh `192.168.144.11/24` nối SIYI Link.
- **WiFi AP (`uap0`)**: SSID: `AP_DRONE` | Pass: `12345678` | Gateway: `192.168.10.1`.
- **GCS Telemetry**: UDP bi-directional `thong.local:14550`.
- **RTSP Camera**: `rtsp://thong.local:8554/camera`.
- **WebRTC Camera**: `http://thong.local:8889/camera`.

---

## 4. BỘ KIỂM THỬ TẬP TRUNG (TEST MATRIX: 33/33 PASSING)
```bash
make test # Chạy toàn bộ 33 Unit & Integration Tests qua CTest trong ~0.6s
```
- `HardwareManagerServerTest` + `SerialScannerTest`: 8 tests.
- `RouterSupervisorTest` + `RouterConfigGeneratorTest`: 6 tests.
- `ConfigEngineTest` + `RollbackGuardTest` + `MetricsTest`: 11 tests.
- `CameraConfigManagerTest` + `CameraFrameProcessorTest`: 3 tests.
- `IpcHardwareRouterIntegrationTest`: 3 tests (IPC UDS thực tế).
- `MavlinkLoopbackTest`: 2 tests (UDP Heartbeat & THACO ID 42014 thực tế).

---

## 5. CÁC LỆNH VẬN HÀNH MAKEFILE THƯỜNG DÙNG
- `make build-native`: Biên dịch toàn bộ mã nguồn C++ và bài test.
- `make test`: Chạy toàn bộ CTest test suite.
- `make build-all` / `make build-core`: Build local 4 core Docker images ARM64 bằng Buildx (--load).
- `make publish-core`: Buildx linux/arm64 --push lên GHCR (SHA + dev tag); `make update` chạy trên Pi: Git pull + Compose pull + up --no-build.
- `make deploy` / `make deploy-fast`: Tiện ích SSH tùy chọn cho Git/GHCR update; archive/rsync chỉ ở `make deploy-offline`.
- `make ps`: Xem trạng thái các container trên Pi.
- `make doctor`: Chạy chẩn đoán tự động tuần tự toàn bộ lỗi hệ thống.

---

## 6. HỆ THỐNG QUY TẮC BẮT BUỘC (.agents/rules/)
- `00-core.md`: Ranh giới dịch vụ, nguồn chân lý, nạp ngữ cảnh.
- `01-networking.md`: Quy định AP-STA, DHCP bind-dynamic, Netplan.
- `02-mavlink.md`: Single source of truth MAVLink, cấm broadcast 14550.
- `03-camera-rtsp.md`: Camera độc lập với Vision, cấu hình `camera.yaml`.
- `04-vision.md`: Pipeline YOLOv8, chạy qua Compose profile `vision`.
- `05-doctor.md`: Ma trận mã lỗi chẩn đoán tự động tuần tự.
- `06-doc-sync-and-rule-governance.md`: Bắt buộc cập nhật README/docs + UML diagram (`sequence.puml`, `system.puml`). **Tuyệt đối hỏi ý kiến người dùng trước khi sửa bất kỳ rule nào**.

*(Nếu cần chi tiết sâu về lịch sử refactor, cấu trúc byte MAVLink, hay giải pháp fix lỗi kinh điển, hãy đọc `PROJECT_CONTEXT_DEEP.md`)*.

## 7. GITHUB / GHCR DEPLOYMENT CONTRACT
- Source/config/model: GitHub. Images: `${REGISTRY:-ghcr.io}/${IMAGE_NAMESPACE}/<image>:${IMAGE_TAG}`; namespace do operator cung cấp.
- Build/publish chỉ trên WSL x86_64 qua `drone-builder`, target `linux/arm64`; Pi không compile.
- Compose production không có `build:`; `cc-agent` dùng binary trong image, không phụ thuộc host binary.
- `mavros` là Compose service, `drone-mavros` là image, `src/mavros` là source. Vision publish base GHCR trước app.
- Private `.env`, target config, WiFi credentials và runtime directory không theo dõi bằng Git; named volumes giữ state. Agent telemetry.env tách khỏi deployment .env.
- Tailscale là profile MANUAL / UNRESOLVED; core workflow chỉ gồm 4 services.
- Update không provisioning hardware/netplan/boot, không apt/reboot/prune; kiểm tra ARM64 và revision labels trước khi up.
- Scripts được kiểm thử bằng mocked Git/Docker tại `tests/test_registry_workflow.py`; không thay thế kiểm tra runtime Pi.
