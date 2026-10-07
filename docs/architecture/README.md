# cc-agent Technical Architecture & Design Documentation

Thư mục này chứa toàn bộ các tài liệu đặc tả kiến trúc kỹ thuật, thiết kế hệ thống chuẩn **SOLID & Modern C++ (C++20)**, phân tích đối sánh công nghiệp và lộ trình nâng cấp cho **`cc-agent` (Drone Companion Agent)**:

---

## 📚 Danh Mục Tài Liệu Kỹ Thuật

| Thứ tự | Tên tài liệu | Nội dung chính |
| :---: | :--- | :--- |
| **01** | [`01_system_architecture_specification.md`](01_system_architecture_specification.md) | **Đặc tả kiến trúc hệ thống C++20 tổng thể:** Đặt ra chuẩn thuần Modern C++20 cho toàn bộ hệ thống; quét toàn bộ submodules; bảng 8 yêu cầu chức năng (FR) và 6 yêu cầu phi chức năng (NFR); **Bộ 5 sơ đồ UML** (Kiến trúc tổng thể, Thành phần C++, Class Diagram HAL, Sequence Diagram 2-Phase, Deployment Diagram). |
| **02** | [`02_device_driver_hal_design.md`](02_device_driver_hal_design.md) | **Thiết kế chi tiết mô hình 3 tầng:** Tầng Platform HAL Driver (Abstract Factory `IPlatformHAL` cho Pi 5 và Jetson), Tầng `HardwareAccessManager` (kiểm soát phân quyền RBAC qua `acl_policy.json`, 2-Phase Lifecycle), Tầng MAVLink Middleware & Extensions (`IMavlinkExtension`). |
| **03** | [`03_dynamic_id_discovery.md`](03_dynamic_id_discovery.md) | **Cơ chế nhận diện thiết bị động bằng ID (Zero-Hardcode):** Thay thế toàn bộ hardcode tên FC/SIYI bằng cơ chế tự động bắt gói tin MAVLink `HEARTBEAT` để lấy `compid` và `type`; người dùng cắm đổi cổng nào hệ thống cũng tự nhận diện chính xác 100%. |
| **04** | [`04_current_drawbacks_and_audit.md`](04_current_drawbacks_and_audit.md) | **Báo cáo phân tích 14 nhược điểm của cc-agent hiện tại:** God Object `MavlinkCore`, Hardcode 2-Link, Busy-polling loop 5ms, Socket in-use race conditions, Unbounded thread creation trong UDS, lỗi bỏ qua `bind()` và rủi ro `SIGPIPE`. |
| **05** | [`05_agricultural_drone_evaluation.md`](05_agricultural_drone_evaluation.md) | **Đánh giá chuyên sâu dưới góc nhìn Drone Nông Nghiệp:** Phân tích ưu/nhược điểm trong điều kiện thực địa khắc nghiệt (nắng gắt 40°C, rung chấn động cơ cánh quạt lớn, hóa chất ăn mòn); quy trình bảo vệ quá nhiệt; luồng Dual-Path (Fast-Path cho điều khiển phun realtime $<5$ms). |
| **06** | [`06_siyi_vs_fc_status_analysis.md`](06_siyi_vs_fc_status_analysis.md) | **Phân tích nguyên nhân gốc rễ sự cố phần cứng SIYI vs FC:** Đo đạc bộ đếm UART kernel `/proc/tty/driver/ttyAMA`, giải thích cơ chế Fallback Dual-Source và hiện tượng PL011 DMA trên Raspberry Pi 5. |
| **07** | [`07_dji_comparison_and_architectural_gaps.md`](07_dji_comparison_and_architectural_gaps.md) | **Đối sánh với DJI (OSDK, PSDK, Agras T40/T50) & 5 Trụ Cột Còn Thiếu:** Đánh giá sự tương đồng với kiến trúc công nghiệp DJI; phân tích khoảng cách công nghệ và đề xuất bổ sung: Hệ thống quản lý sức khỏe (HMS), Đồng bộ thời gian vi giây (TimeSync), Giao diện động (Dynamic UI Widgets), An toàn bay thông minh (Smart Agri-Failsafe), Hộp đen thống nhất (Unified Blackbox). |
| **08** | [`08_docker_architecture_and_master_requirements.md`](08_docker_architecture_and_master_requirements.md) | **Thiết kế kiến trúc Docker chuẩn hóa & Tổng hợp toàn bộ yêu cầu dự án:** Phân rã 5 micro-daemons theo DDD, cơ chế Zero-Copy Shared Memory (`/dev/shm`), hạn ngạch cgroups (CPU/RAM quotas), phân quyền Least Privilege (loại bỏ `privileged: true`), file `docker-compose.yml` chuẩn sản xuất, và Bảng Master Requirements (REQ-DOM, REQ-HW, REQ-SW, REQ-ARC, REQ-SAF). |
| **09** | [`09_container_strategy_and_ros2_longterm_roadmap.md`](09_container_strategy_and_ros2_longterm_roadmap.md) | **Đánh giá container hiện tại, khả năng mở rộng ROS 2 & Lộ trình dài hạn:** Phân tích điểm nghẽn của docker-compose hiện tại, mô hình 2 Vùng (Mission-Critical Core vs High-Level Autonomy), kiến trúc tích hợp ROS 2 tối ưu qua Micro-XRCE-DDS thay thế MAVROS, và lộ trình phát triển 4 giai đoạn tới Jetson Orin. |
| **10** | [`10_mavlink_vs_micro_xrce_dds_dual_bus_architecture.md`](10_mavlink_vs_micro_xrce_dds_dual_bus_architecture.md) | **Kiến trúc giao tiếp kép MAVLink UART vs Micro-XRCE-DDS:** Khẳng định bắt buộc tách luồng, so sánh 3 phương án kết nối vật lý giữa Cube Orange Plus và Raspberry Pi 5, và chứng minh giải pháp tối ưu nhất là **1 UART (TELEM2) cho MAVLink + 1 Cáp USB Type-C cho Micro-XRCE-DDS** (băng thông > 10MB/s, chống nghẽn, cách ly an toàn). |
| **11** | [`11_baudrate_limits_and_mavlink_vs_dds_selection_guide.md`](11_baudrate_limits_and_mavlink_vs_dds_selection_guide.md) | **Phân tích giới hạn baudrate UART & Lựa chọn duy nhất MAVLink vs DDS:** Phân tích vật lý méo xung do tải ký sinh RC và nhiễu điện từ động cơ ESC khi nâng baudrate lên 2M-3M; lý do bắt buộc phải chọn MAVLink nếu phải chọn duy nhất 1 giao thức cho drone nông nghiệp thương mại. |

---

## 🏗️ Nguyên Tắc Thiết Kế Cốt Lõi (Core Engineering Tenets)
1. **Thuần Modern C++20:** Không dùng runtime Python hoặc shell script cho các tiến trình nền của Companion Computer.
2. **Tuân thủ SOLID:**
   - **SRP:** Tách biệt Gateway, Dispatcher, Hardware Access Manager và các Driver.
   - **OCP:** Mở rộng linh hoạt thiết bị và cảm biến mới chỉ bằng cách thêm Driver/Extension kế thừa Interface, không sửa code lõi.
   - **LSP & DIP:** Phụ thuộc vào Interface trừu tượng, 100% kiểm thử được (Mockable) trên môi trường x86 mà không cần root hay bo mạch nhúng thật.
3. **An toàn bay & Phân quyền (Security & Safety):** Mọi lệnh cấu hình phần cứng đều phải được thẩm định quyền hạn người gửi (`sysid`, `compid`) trước khi thực thi.
