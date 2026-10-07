# T3 Spike trên WSL — Kế hoạch

Bead: `THACO_Drone-083.4`. Spec §7 P2, §5.10, ngưỡng §12. Code trong thư mục này là code vứt đi; kết quả đo mới là sản phẩm, ghi vào bead.

## Phạm vi

| # | Mục T3 (plan) | Làm trên WSL? | Cách làm |
|---|---|---|---|
| 1 | `take()` trên callback group không gắn executor | Có | `ros2_api/spike_ros2_api take` |
| 2 | `AsyncParametersClient` sang node vắng mặt / có mặt | Có (hai process, chưa hai container) | `spike_ros2_api param_absent`, `param_present` |
| 3 | Kích thước image, RSS idle node C++ / rclpy trên `ros:jazzy-ros-core` arm64 | Chỉ RSS x86 để tham khảo; số arm64 chờ Pi | `spike_ros2_api rss`, ghi chú |
| 4 | Shm ring: CPU, tỉ lệ rớt, frame xé | Có (writer C++ + reader Python, hai process, tmpfs `/dev/shm`) | `shm_ring/` |
| 5 | `tcpdump` 60 s, 0 gói UDP 7400–7600 | Không, cần Pi và compose thật | để T3 bù |
| 6 | Deploy: `registry:2` + `ssh -R` vs `docker save` | Không, cần Pi | để T3 bù |

Mục 3 (số arm64), 5, 6 giữ bead T3 mở ở dạng "phần Pi còn thiếu". Gate T5 dựa trên mục 1, 2, 4 đo được ở đây; nếu số đo trên Pi không đạt thì quay lại sửa T5.

## Spike 1: `take()` và `AsyncParametersClient`

Tiêu chí đạt:
- `take()`: publish N=100 message, gọi `take()` đủ 100 lần đều trả `true` với đúng giá trị theo thứ tự; lần gọi thứ 101 trả `false`; mỗi lần `take()` rỗng dưới 1 ms. Subscription tạo trên callback group `automatically_add_to_executor_with_node=false` và không có executor nào spin.
- Param client, node vắng: `service_is_ready()` trả `false`, mỗi lần gọi dưới 10 ms, không chặn.
- Param client, node có mặt (process riêng): `service_is_ready()` lên `true` trong 5 s, `get_parameters` trả đúng giá trị.

## Spike 2: shm ring chống xé frame (spec §5.10)

Định dạng file `/dev/shm/spike_pool`:
- header 64 byte: magic `CCFP`, version, slot_count=3, width=640, height=480, slot_size.
- mỗi slot căn 64 byte: `uint64 seq`, `uint64 timestamp_ns`, color 640×480×3 (BGR), depth 640×480×2 (16UC1).
- Seqlock: ghi `seq` lẻ (release), ghi dữ liệu, ghi `seq` chẵn (release). Frame thứ n có `seq = 2n`.
- Descriptor `(slot, seq)` gửi bằng UDP loopback 8 byte, thay cho `cc_msgs/FrameReady`. Reader chỉ giữ descriptor mới nhất (giả lập QoS depth 1).

Reader Python, cho mỗi descriptor mới nhất:
1. ngủ ngẫu nhiên 0–400 ms (giả lập YOLO);
2. đọc `seq` slot, khác descriptor thì bỏ (`dropped_seq`);
3. `np.frombuffer(...).copy()` cho color và depth, đo thời gian;
4. đọc lại `seq`, khác thì bỏ (`dropped_seq`);
5. kiểm nội dung: mọi byte color và depth phải bằng `(seq/2) & 0xFF`; sai là `torn_escaped`.

Hai kịch bản:
- **spec**: 10 Hz, 3 slot, 600 frame (60 s). Đạt khi `torn_escaped == 0`, copy trung bình dưới 0,5 ms.
- **stress** (kiểm độ nhạy của test): writer 2000 Hz, 3 slot, 20000 frame. Chạy hai lần: tắt kiểm `seq` (`--no-seqcheck`) phải thấy `torn_escaped > 0` (chứng minh test bắt được xé frame), bật kiểm `seq` phải `torn_escaped == 0`.

Đo thêm: CPU writer (`clock()`), CPU reader (`time.process_time()`), tỉ lệ rớt = 1 − frame xử lý / frame ghi.

## Gate và chốt

1. `./run_all.sh` in bảng PASS/FAIL.
2. Ghi số đo vào bead: `bd update THACO_Drone-083.4 --notes "..."`.
3. Tất cả PASS thì T5 được phép đi tiếp theo hướng WSL; ghi rõ ba mục còn nợ Pi. Không đạt thì dừng, hỏi người dùng.
4. Không tự `bd close` T3 vì còn nợ Pi; người dùng quyết.

## Cấu trúc

```text
spike_ros2/
├── README.md
├── PLAN.md
├── run_all.sh
├── ros2_api/        # Spike 1 (C++, rclcpp)
│   ├── CMakeLists.txt
│   └── spike_ros2_api.cpp
└── shm_ring/        # Spike 2
    ├── writer.cpp
    ├── reader.py
    └── run.sh
```
