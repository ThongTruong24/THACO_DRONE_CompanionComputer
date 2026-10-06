# spike_ros2 — T3 spike trên WSL

Bead `THACO_Drone-083.4`. Kế hoạch chi tiết: [PLAN.md](PLAN.md). Code vứt đi, không đưa vào `Companion_Computer`.

## Chạy

```bash
cd /home/lnh/THACO_Drone/Companion_Computer/tests/spike_ros2
./run_all.sh          # khoảng 2 phút, in PASS/FAIL từng bước
```

Cần: ROS 2 Jazzy (`/opt/ros/jazzy`), `g++`, `cmake`, `python3-numpy`.

Chạy riêng từng phần:

```bash
source /opt/ros/jazzy/setup.bash
cmake -S ros2_api -B ros2_api/build && cmake --build ros2_api/build
ros2_api/build/spike_ros2_api take            # take() trên callback group không gắn executor
ros2_api/build/spike_ros2_api param_absent    # param client, node vắng
shm_ring/run.sh spec 10 600 --expect-torn zero --max-copy-ms 0.5
```

## Nội dung

| Thư mục | Kiểm gì |
|---|---|
| `ros2_api/` | `take()` không chặn, `AsyncParametersClient::service_is_ready()` khi node vắng / có mặt |
| `shm_ring/` | shm ring 3 slot + seqlock theo spec §5.10: writer C++ → reader Python, tỉ lệ rớt, frame xé, thời gian copy, CPU |

Bốn kịch bản shm: `spec` (đạt chuẩn), `naive` (không copy, phải thấy hỏng), `stress-noseq` (bỏ kiểm seq, phải thấy xé), `stress-seq` (có kiểm seq, phải 0 xé). Ba kịch bản sau chứng minh test đủ nhạy để bắt lỗi.

## Chưa làm ở đây (nợ Pi)

- Kích thước image và RSS idle trên `ros:jazzy-ros-core` arm64.
- `tcpdump` 60 s: 0 gói UDP 7400–7600.
- So sánh deploy `registry:2` + `ssh -R` với `docker save`.

## Kết quả (WSL, x86_64, 2026-10-05)

| Mục | Kết quả |
|---|---|
| `take()` | PASS: 100/100 đúng thứ tự, `take()` rỗng 0,006 ms trả `false` |
| Param client, node vắng | PASS: `service_is_ready()` = `false`, tệ nhất 0,018 ms |
| Param client, node có mặt (process riêng) | PASS: `service_is_ready()` = `true`, `get_parameters` trả 42 |
| RSS idle node C++ (x86) | 16 MB (chỉ tham khảo, số arm64 chờ Pi) |
| shm ring `spec` (10 Hz, 600 frame) | PASS: 0 frame xé, copy trung bình 0,257 ms (tối đa 1,84 ms), writer 0,2% một core |
| shm ring `naive` (đối chứng, không copy) | 50/152 frame hỏng: dùng thẳng vùng nhớ là sai |
| shm ring `stress-noseq` (2000 Hz, bỏ kiểm seq) | 154 frame xé lọt qua: test đủ nhạy |
| shm ring `stress-seq` (2000 Hz, có kiểm seq) | PASS: 0 xé, 85 frame bị loại đúng bởi seq |

Tỉ lệ rớt ở kịch bản `spec` là 50,8%. Con số này là do reader ngủ trung bình 200 ms trong khi writer ghi mỗi 100 ms, nên cứ hai descriptor thì reader chỉ xử lý một. Đây là hành vi QoS depth 1 mong muốn, không phải mất dữ liệu.

### Phát hiện: discovery DDS giữa process không chạy trên WSL

`lo` của WSL không có cờ `MULTICAST`. Với cấu hình mặc định (Fast DDS, và cả Cyclone với `ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST`), hai process khác nhau không thấy nhau: `ros2 node list` rỗng, `service_is_ready()` không bao giờ lên `true`. Cùng một process thì vẫn chạy bình thường.

Cách chạy được: Cyclone với discovery unicast trên `lo`, xem [ros2_api/cyclonedds_lo.xml](ros2_api/cyclonedds_lo.xml). `run_all.sh` đã đặt `RMW_IMPLEMENTATION` và `CYCLONEDDS_URI` sẵn.

Việc cần làm ở T5 và T13: compose của các container edge-* phải có cấu hình discovery rõ ràng cho `lo`, không dựa vào multicast mặc định. Cần kiểm lại trên Pi, vì `lo` của Linux thường cũng không có cờ `MULTICAST`.

Ghi chú nhỏ: override parameter chỉ hiện ra khi node khai báo nó. Server dùng `automatically_declare_parameters_from_overrides(true)`.

