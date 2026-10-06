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
| **Companion Agent & HW Registry** | `src/cc-agent` | `cc-agent:latest` | `/run/drone/hw_manager.sock`<br>`/run/drone/cc_agent.sock`<br>UDP `:14600` (CompID 191) | Quản lý độc quyền phần cứng (HardwareRegistry), Transaction 2-pha, rollback guard 30s |
| **MAVLink Router** | `src/mavlink-router-controller` | `mavlink-router-controller:latest` | `/run/drone/router.sock`<br>UDP `:14550`, `:14541`, `:14600` | Điều phối MAVLink, standby mode, reload IPC |
| **Networking** | `src/drone-networking` | `drone-networking:latest` | AP `uap0` (192.168.10.1)<br>DNS 53, mDNS 5353 (`thong.local`) | AP-STA Concurrency, DHCP `dnsmasq`, mDNS |
| **Camera RTSP** | `src/camera-stream-controller` | `camera-stream-controller:latest` | RTSP `:8554/camera`<br>WebRTC `:8889/camera` | Thu hình RealSense/V4L2, tăng tốc NEON SIMD |
| **AI Vision** | `src/drone-vision` | `drone-vision:latest` (profile `vision`) | RTSP `:8554/yolo` | Suy luận YOLOv8, phát hiện vật thể |
| **AI Vision v1** | `src/modules/vision_v1` | `edge-vision-v1:latest` (profile `vision-v1`) | RTSP input `:8554/camera`, FramePool `/run/frame_pool`, RTSP output `:8554/yolo` | Video độc lập YOLO qua latest `DetectionSnapshot`; không chạy đồng thời legacy vision |
| **MAVROS** | `src/mavros` | `drone-mavros:latest` (profile `mavros`) | MAVLink UDP `:14541` (Local socket) | Cầu nối ROS 2 Jazzy, telemetry plugin |

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

## 4. BỘ KIỂM THỬ TẬP TRUNG (TEST MATRIX: 82/82 PASSING)
```bash
make test # Chạy toàn bộ 82 Unit & Integration Tests qua CTest trong ~16s
```
> Refactor cc-agent (đang tiến hành): `WorkerQueue` (4, hàng đợi bounded), `RouterSockClientParse` (5, IPC router persistent + framed JSON), `LinkClassifier` (4, SIYI nhận diện theo compid bất kể sysid — sửa lỗi SIYI OFFLINE), `ConfigEngineSafeParse` (1, `.env` hỏng không crash).
- `HardwareManagerServerTest` + `SerialScannerTest` + `BaudFromTermiosSpeed` + `GetUartLineBaud`: 13 tests (gồm đọc baud thực của UART qua `tcgetattr`).
- `RouterSupervisorTest` + `RouterConfigGeneratorTest`: 6 tests.
- `ConfigEngineTest` + `RollbackGuardTest` + `MetricsTest`: 13 tests (gồm kiểm thử khóa `.env` chuẩn `DRONE_SERIAL_PORT`/`SIYI_SERIAL_PORT` khớp với router & mavros).
- `LinkStats`: 7 tests (telemetry FC/SIYI lấy rate/bytes/loss/status từ delta kernel UART TIOCGICOUNT; baud lệch hiện thành RX loss).
- `ParamRegistry`: 7 tests (surface cấu hình CC qua PARAM_EXT; set thất bại giữ nguyên giá trị thật để readback — không rollback).
- `CameraConfigManagerTest` + `CameraFrameProcessorTest`: 3 tests.
- `IpcHardwareRouterIntegrationTest`: 3 tests (IPC UDS thực tế).
- `UdsHubAddressingTest`: 3 tests (hello-handshake, gửi có địa chỉ theo subsystem + fallback broadcast, liveness).
- `MavlinkLoopbackTest`: 2 tests (UDP Heartbeat & THACO ID 42014 thực tế).
- `CcTelemetryLinksRoundtrip`: 2 tests (ánh xạ trường `CC_TELEMETRY_LINKS` mà panel Config của QGC giải mã).
- `ParamExtWire`: 3 tests (encode PARAM_EXT đúng dialect của agent; giá trị nhị phân có byte 0x00 đầu vẫn roundtrip).

---

## 5. CÁC LỆNH VẬN HÀNH MAKEFILE THƯỜNG DÙNG
- `make build-native`: Biên dịch toàn bộ mã nguồn C++ và bài test.
- `make test`: Chạy toàn bộ CTest test suite.
- `make build-all`: Build 5 core Docker images ARM64 bằng buildx.
- `make deploy`: Deploy toàn bộ 5 core containers lên Raspberry Pi (`thong.local`).
- `make deploy-fast`: Deploy nhanh khi image đã có sẵn.
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
