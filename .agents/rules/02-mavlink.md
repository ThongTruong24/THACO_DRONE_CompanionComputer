# MAVLink rules

## Single Source of Truth for MAVLink Packets (BẮT BUỘC)
- **Tất cả gói tin MAVLink custom và chuẩn CHỈ ĐƯỢC PHÉP cập nhật tại:** `$HOME/Mavlink/custom/thaco_common.xml`.
- **TUYỆT ĐỐI KHÔNG** sửa tay trực tiếp các file headers (`.h`, `.hpp`) hoặc file Python trong `Drone` hay `THACOGroundControl`.
- **TUYỆT ĐỐI KHÔNG** tạo file XML định nghĩa rời rạc trong từng repo.
- Sau khi chỉnh sửa `thaco_common.xml`, **BẮT BUỘC** chạy lệnh đồng bộ:
  ```bash
  $HOME/Mavlink/sync_all.sh
  ```
  Script này sẽ tự động sinh code C++ và Python, kiểm tra schema và đồng bộ thẳng vào cả `Drone` và `THACOGroundControl`.
- Khi cần cập nhật chuẩn MAVLink mới nhất từ Upstream (PX4/ArduPilot): Chạy `$HOME/Mavlink/scripts/update_upstream.sh`, hệ thống sẽ cập nhật thư mục `standard/` mà không gây xung đột hay ghi đè lên dialect của THACO.

---

- FC uses `${DRONE_SERIAL_PORT}` and `${DRONE_BAUD_RATE}`; SIYI is `/dev/ttyAMA0` at `115200`. Serial devices must be validated through `cc-agent`'s `HardwareRegistry` (`/run/drone/hw_manager.sock`).
- If FC UART port is absent, `mavlink-router-controller` must run in **Graceful Standby Mode** (UDP endpoints active without crashing).
- The authoritative GCS path is the bidirectional UDP server `0.0.0.0:14550`. QGroundControl must send an initial UDP packet to the Pi gateway (`192.168.10.1:14550`) so the router learns its return address.
- **Never configure a broadcast endpoint on UDP `14550`** including `192.168.10.255:14550`, `10.x.x.255:14550`, `LAN_wlan0`, or `LAN_uap0` when MAVLink Router binds `0.0.0.0:14550` under `network_mode: host`. The broadcast loops into the local UDP server and creates packet amplification.
- For a passive GCS that cannot send first, use only a validated direct-unicast `GCS_IP:14550` endpoint. The address must be a current client DHCP address, never a broadcast or multicast address.
- Treat an abnormal jump in `QGC_UDP_Server` counters, high sequence loss, or degraded SSH/Wi-Fi latency as a possible UDP loop. Inspect the rendered `/etc/mavlink-router/main.conf` before changing UART baud or wiring.
- Deploy runtime configuration updates with `docker compose up -d --force-recreate --no-deps mavlink-router-controller`; a plain `docker compose restart` may retain stale rendered state.
- Verify after every telemetry change: container health, FC UART message/loss counters, absence of broadcast/LAN endpoints in rendered config, udp `14550` socket, and bounded packets on `uap0` with `tcpdump`. A listening socket alone is insufficient.
- SIYI UART loss is a separate serial-quality concern; isolate or disable its endpoint during diagnosis rather than conflating it with FC/Wi-Fi telemetry.
- See `src/mavlink-router-controller/README.md` for QGC configuration and deployment commands.

- MAVROS must consume only the dedicated local Router server `127.0.0.1:14541` from source port `14542`; never let MAVROS open FC UART or reuse TCP proxy endpoint `:14540`.
