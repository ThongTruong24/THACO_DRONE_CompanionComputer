# 06 — ĐỒNG BỘ TÀI LIỆU VÀ QUẢN TRỊ QUY TẮC HỆ THỐNG (DOC SYNC & RULE GOVERNANCE)

Quy tắc này là bắt buộc (MANDATORY) đối với AI Assistant và mọi contributor khi thực hiện bất kỳ thay đổi nào trong repository `Drone`.

---

## 1. NGUYÊN TẮC KÍCH HOẠT (TRIGGER CONDITIONS)

Quy trình đồng bộ tài liệu và kiểm tra quy tắc BẮT BUỘC được kích hoạt ngay khi có một trong các hành động sau:
1. **Thay đổi Mã nguồn (Source Code)**: Thêm mới, chỉnh sửa logic, tái cấu trúc (refactor) hoặc xóa code C++, Python, Shell scripts.
2. **Thay đổi Cấu hình (Configuration)**: Sửa đổi `.env`, `docker-compose.yml`, các file cấu hình `*.yaml`, `*.json`, `*.conf`, `netplan`, `mediamtx.yml`, `dnsmasq.conf`, `hostapd.conf`.
3. **Thay đổi Biên dịch & Triển khai (Build & Deploy)**: Sửa đổi `Makefile`, `CMakeLists.txt`, `Dockerfile`, `scripts/deploy.sh`, `scripts/doctor.sh`, hoặc kịch bản CI/CD.
4. **Thay đổi Giao tiếp & Mạng (Communication & Networking)**: Thay đổi cổng mạng (Ports: 14550, 14541, 8554, 53, 5760...), Unix Domain Socket (`/run/drone/*.sock`), địa chỉ IP, hoặc MAVLink message IDs.

---

## 2. MA TRẬN ĐỒNG BỘ TÀI LIỆU (DOCUMENTATION MAPPING MATRIX)

Khi một thành phần kỹ thuật thay đổi, các tài liệu tương ứng sau đây BẮT BUỘC phải được rà soát và cập nhật:

| Thành phần thay đổi | File / Module bị tác động | Tài liệu BẮT BUỘC rà soát & cập nhật |
| :--- | :--- | :--- |
| **Kiến trúc toàn hệ thống, Deploy, Makefile** | `docker-compose.yml`, `Makefile`, `scripts/*` | `README.md` (root), `AGENTS.md` |
| **Phần cứng & Serial Scanner** | `src/cc-agent/` | `src/cc-agent/README.md`, `src/cc-agent/docs/` |
| **Định tuyến MAVLink, Port 14550/14541** | `src/mavlink-router-controller/` | `src/mavlink-router-controller/README.md`, `src/mavlink-router-controller/docs/` |
| **Bộ não điều khiển & Telemetry** | `src/cc-agent/` | `src/cc-agent/README.md`, `src/cc-agent/docs/` |
| **Mạng WiFi AP, DHCP, Netplan, IP** | `src/drone-networking/` | `src/drone-networking/README.md`, `src/drone-networking/docs/` |
| **Camera RealSense, RTSP, MediaMTX** | `src/camera-stream-controller/` | `src/camera-stream-controller/README.md`, `src/camera-stream-controller/docs/` |
| **AI Vision & YOLO Pipeline** | `src/drone-vision/` | `src/drone-vision/README.md`, `src/drone-vision/docs/` |
| **MAVROS ROS 2 Bridge** | `src/mavros/` | `src/mavros/README.md`, `src/mavros/docs/` |

---

## 3. QUY ĐỊNH HỒ SƠ TÀI LIỆU VÀ SƠ ĐỒ UML TRONG MỖI MODULE (BẮT BUỘC)

> [!IMPORTANT]
> **Mỗi module tại `src/<module>/` bắt buộc phải có đầy đủ:**
> 1. File `README.md` đạt chuẩn chất lượng cao.
> 2. Thư mục `docs/` chứa các sơ đồ UML kỹ thuật.
> 3. **Nếu module nào thiếu bất kỳ file nào trong số trên, Agent PHẢI TỰ ĐỘNG KHỞI TẠO MỚI ("thiếu thì tự thêm")**.

### Yêu cầu sơ đồ UML trong `src/<module>/docs/`:
- **`sequence.puml` (Sơ đồ tuần tự)**: Mô tả luồng chạy từ khi khởi động, handshake, giao thức IPC/MAVLink, tương tác thread và kịch bản kết thúc/lỗi.
- **`system.puml` (Sơ đồ kiến trúc phân hệ)**: Mô tả cấu trúc component, các luồng thread nội bộ, cổng I/O phần cứng (`/dev/tty*`, Camera), Unix Domain Sockets (`/run/drone/*.sock`), và các socket mạng.

### Tiêu chuẩn chất lượng README (README Quality Score Checklist):
Mỗi file `README.md` (cả root và từng module) phải đáp ứng 7 tiêu chí:
1. **Badges**: Hiển thị trạng thái build, C++20, ARM64, License.
2. **Table of Contents**: Mục lục điều hướng nhanh có link neo.
3. **Installation & Build**: Hướng dẫn biên dịch Native C++ và Docker ARM64.
4. **Usage & Configuration**: Lệnh chạy, bảng biến môi trường/file cấu hình, ví dụ cụ thể.
5. **Diagrams**: Nhúng trực tiếp cú pháp **Mermaid** để người xem có thể nhìn thấy sơ đồ trực quan ngay trên GitHub / IDE Preview.
6. **Contributing**: Quy chuẩn đóng góp code và kiểm thử.
7. **License**: Ghi rõ giấy phép mã nguồn mở (MIT License).

---

## 4. QUY TRÌNH TÙY CHỌN ĐỒNG BỘ (INTERACTIVE OPTIONS WORKFLOW)

Sau khi hoàn thành việc chỉnh sửa code/config, Agent thông báo rõ ràng danh sách tài liệu bị ảnh hưởng kèm 3 tùy chọn:
1. **(Khuyến nghị) Cập nhật ngay tất cả tài liệu trên**: Tự động cập nhật nội dung chính xác kèm tạo mới UML nếu thiếu.
2. **Chỉ cập nhật tài liệu được chỉ định**: Người dùng chọn file muốn cập nhật.
3. **Tạm hoãn (Bỏ qua đợt này)**: Khi đang debug thử nghiệm dở dang, sẽ cập nhật sau.

---

## 5. CHỐT CHẶN BẢO VỆ QUY TẮC: HỎI Ý KIẾN TRƯỚC KHI SỬA RULE (RULE GOVERNANCE)

> [!CAUTION]
> **ĐIỀU KHOẢN TỐI THƯỢNG:**
> 1. **TUYỆT ĐỐI CẤM** AI Assistant tự ý chỉnh sửa, tạo mới, xóa bỏ hoặc nới lỏng các quy định trong thư mục `.agents/rules/`, `AGENTS.md`, hoặc `GEMINI.md` mà không có sự đồng ý rõ ràng của người dùng.
> 2. Khi phát hiện mã nguồn hoặc yêu cầu mới làm cho một rule hiện tại bị mâu thuẫn hoặc lỗi thời:
>    - Giải thích rõ nguyên nhân cho người dùng.
>    - Hiển thị dự thảo nội dung rule mới hoặc phần diff.
>    - Hỏi ý kiến trực tiếp người dùng và CHỈ KHI nhận được đồng ý mới được phép chỉnh sửa.
