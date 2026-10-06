# Kế hoạch Phân tích: Trạng thái SIYI Link OFFLINE so với FC ONLINE

## 1. Mục tiêu
Giải thích tường tận lý do tại sao trên giao diện QGroundControl:
- **Cube Orange Plus (FC)** hiển thị **Link Status: ONLINE** (đèn xanh).
- **SIYI Gimbal & Camera** hiển thị **Link Status: OFFLINE** (đèn đỏ).
- Phân tích sự tương đồng và khác biệt trong luồng dữ liệu giữa hai bên ở tầng kernel và tầng mã nguồn (không can thiệp chỉnh sửa mã nguồn).

---

## 2. Bằng chứng thực tế thu thập từ Drone (`thong@drone.local`)

### A. Bộ đếm phần cứng UART của Linux Kernel (`/proc/tty/driver/ttyAMA`)
Kiểm tra số byte thực tế truyền nhận trên các thanh ghi UART vật lý của Raspberry Pi trong 2 giây liên tiếp:

| Cổng UART | Thiết bị | TX (Byte truyền đi) | RX (Byte nhận về) | Tốc độ nhận thực tế |
| :--- | :--- | :--- | :--- | :--- |
| **`/dev/ttyAMA4`** | **Cube Orange Plus (FC)** | 63,302,196 $\rightarrow$ 63,303,670 | 189,899,260 $\rightarrow$ 189,904,031 | **+4,771 bytes/2s (~2.4 KB/s)** |
| **`/dev/ttyAMA0`** | **SIYI Telemetry** | 252,286,885 $\rightarrow$ 252,293,088 | 157,913 $\rightarrow$ 157,913 | **0 byte (Đứng yên tuyệt đối)** |

> [!IMPORTANT]
> - Chân **TX của cả hai cổng** đều đang phát dữ liệu (do `mavlink-router` đẩy dữ liệu ra).
> - Chân **RX của Cube FC (`ttyAMA4`)** đang nhận dữ liệu MAVLink dồn dập (~2.4 KB/s).
> - Chân **RX của SIYI (`ttyAMA0`)** **hoàn toàn không nhận được bất kỳ tín hiệu điện áp nào** (số byte RX không tăng dù chỉ 1 byte, số framing error `fe` cũng không tăng).

---

### B. Kiểm tra luồng gói tin MAVLink thực tế qua Router
Bắt gói tin MAVLink thực tế trên luồng mạng nội bộ của drone (`127.0.0.1:5760`):

```text
[Cube FC] msgid=30  sys=1 comp=1: 46 packets (ATTITUDE)
[Cube FC] msgid=31  sys=1 comp=1: 30 packets (ATTITUDE_QUATERNION)
[Cube FC] msgid=24  sys=1 comp=1: 15 packets (GPS_RAW_INT)
[Cube FC] msgid=65  sys=1 comp=1: 15 packets (RC_CHANNELS)
[Cube FC] msgid=74  sys=1 comp=1: 12 packets (VFR_HUD)
[Cube FC] msgid=0   sys=1 comp=1:  3 packets (HEARTBEAT định kỳ 1Hz)
...
[SIYI]    compid=154 (Gimbal) / compid=100 (Camera) / compid=68 (Radio): 0 GÓI TIN
```

---

## 3. Phân tích Logic mã nguồn: Vì sao FC ONLINE mà SIYI lại OFFLINE?

Trong bức ảnh chụp màn hình của bạn:
- FC: `TX Rate: 0 B/s, RX Rate: 0 B/s`, nhưng **Link Status: ONLINE**.
- SIYI: `TX Rate: 0 B/s, RX Rate: 0 B/s`, và **Link Status: OFFLINE**.

Tại sao cùng hiển thị `0 B/s` mà trạng thái đèn lại khác nhau?

### Cơ chế đánh giá trạng thái trong [`link_stats.hpp`](file:///home/lnh/THACO_Drone/Companion_Computer/src/cc-agent/include/link_stats.hpp#L75-L120):
```cpp
// 1. Đối với Flight Controller:
fc_active = fb.fc_router_online || fb.fc_online_hint;
fc_active_ticks_ = tick_window(fc_active_ticks_, fc_active);
f.fc_status = (fc_active_ticks_ > 0) ? 2 : 0; // 2 = ONLINE, 0 = OFFLINE

// 2. Đối với SIYI:
siyi_active = fb.siyi_router_online || fb.siyi_mavlink_online;
siyi_active_ticks_ = tick_window(siyi_active_ticks_, siyi_active);
f.siyi_status = (siyi_active_ticks_ > 0) ? 2 : 0;
```

### Điểm khác biệt mấu chốt:
1. **Tại thời điểm chụp ảnh**, bộ đếm thông kê từ tiến trình `mavlink-router` chưa nạp xong pipe nên cả `fc_router_online` và `siyi_router_online` đều trả về 0 B/s.
2. **Tuy nhiên, hệ thống có cơ chế dự phòng Fallback**:
   - `fb.fc_online_hint = guard_.is_fc_online()`:
     Trong [`mavlink_core.cpp`](file:///home/lnh/THACO_Drone/Companion_Computer/src/cc-agent/src/mavlink_core.cpp#L539-L546), mỗi khi nhận được gói tin Heartbeat hoặc telemetry từ FC (`sysid=1, compid=1`), hàm `guard_.record_fc_heartbeat()` được gọi. 
     Do FC đang phát hàng trăm gói MAVLink/giây qua UART4, `guard_.is_fc_online()` **luôn luôn trả về `true`**.
     $\rightarrow$ `fc_active` lập tức bằng **`true`**, FC chuyển sang đèn xanh **ONLINE**.
   - `fb.siyi_mavlink_online`:
     Hàm kiểm tra `siyi_mavlink_online` yêu cầu phải có ít nhất 1 gói MAVLink trong vòng 3 giây từ các component SIYI (`compid == 154` Gimbal, `100` Camera, hoặc `68` Radio).
     Vì chân RX của `ttyAMA0` không hề có tín hiệu điện áp từ SIYI, `siyi_mavlink_online` **luôn luôn là `false`**.
     $\rightarrow$ Cả 2 nhánh kiểm tra của SIYI đều bằng `false`, SIYI giữ nguyên đèn đỏ **OFFLINE**.

---

## 4. Bảng So sánh Tổng hợp

| Thành phần | Cube Orange Plus (FC) | SIYI Gimbal & Camera |
| :--- | :--- | :--- |
| **Cổng vật lý** | `/dev/ttyAMA4` (UART4) | `/dev/ttyAMA0` (UART0) |
| **Tín hiệu RX phần cứng** | **Có** (~2.4 KB/s liên tục) | **Không** (0 B/s, đứng yên) |
| **Gói tin MAVLink đến CC** | Hàng chục gói/giây (`sys=1, comp=1`) | **0 gói tin** (`compid=154/100/68`) |
| **Router Online Flag** | Phụ thuộc router stats | Phụ thuộc router stats |
| **Fallback MAVLink Hint** | **Active (TRUE)** do nhận được Heartbeat | **Inactive (FALSE)** do không có gói nào |
| **Kết luận Status UI** | **ONLINE (Xanh)** | **OFFLINE (Đỏ)** |

---

## 5. Nguyên nhân thực tế tại phần cứng của SIYI và các bước kiểm tra khuyến nghị

Vì chân RX của `/dev/ttyAMA0` trên Raspberry Pi đang nhận **chính xác 0 Byte**, vấn đề nằm ở đường truyền vật lý hoặc cấu hình phát tín hiệu của thiết bị SIYI:

1. **Đấu nối dây tín hiệu (Wiring)**:
   - Chân **TX của thiết bị SIYI** (Air Unit / Gimbal) phải nối vào chân **RX của Raspberry Pi** (GPIO 15 - Pin 10 trên header 40-pin).
   - Kiểm tra xem dây có bị cắm nhầm thành **TX sang TX**, hoặc lỏng đầu giắc JST-GH, đứt ngầm dây tín hiệu hay không.
   - **Dây Mass (GND)**: Thiết bị SIYI và Raspberry Pi **bắt buộc phải nối chung dây GND** để có cùng điện thế tham chiếu.
2. **Nguồn cấp thiết bị SIYI**:
   - Kiểm tra đèn LED trạng thái trên Air Unit / Gimbal SIYI: Đèn nguồn và đèn liên kết RF có sáng bình thường hay không.
3. **Cấu hình giao thức cổng trên phần mềm SIYI PC Assistant**:
   - Cắm Air Unit SIYI vào máy tính qua cổng Type-C và mở **SIYI PC Assistant**:
     - Kiểm tra cổng Telemetry đang chọn chế độ nào: Nếu đang để **SIYI SDK** (chuỗi mã `0x55 0x66`), nó không phát MAVLink nên `mavlink-router` sẽ không xử lý.
     - Cần cấu hình cổng Telemetry của SIYI sang chế độ **MAVLink** (tốc độ `115200` hoặc `57600` tương ứng).
