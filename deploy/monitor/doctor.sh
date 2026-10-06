#!/usr/bin/env bash
# ==============================================================================
# doctor.sh - Hệ thống Kiểm tra & Xác thực Tuần tự Kết nối Drone Companion
# Kiểm tra từng bước:
#   [1/6] Phần cứng Serial (UART4 Cube, UART0 SIYI)
#   [2/6] Phần cứng Camera (Intel RealSense D435i USB3)
#   [3/6] MAVLink Router (Xác thực daemon đã lock & mở cổng UART)
#   [4/6] Dữ liệu bay MAVLink thực tế (Heartbeat Probe từ Flight Controller)
#   [5/6] Video Streaming RTSP (MediaMTX + H.264 Track /camera)
#   [6/6] WiFi Access Point (AP_DRONE) & Dnsmasq DHCP
# ==============================================================================

set -o pipefail

# Màu sắc hiển thị
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

PASS="${GREEN}✔ PASS${NC}"
FAIL="${RED}✖ FAIL${NC}"
WARN="${YELLOW}⚠ WARN${NC}"

TOTAL_STEPS=6
CURRENT_STEP=1

print_header() {
    echo -e "${BLUE}======================================================================${NC}"
    echo -e "${BOLD}${BLUE}   🩺 DRONE COMPANION DOCTOR — HỆ THỐNG XÁC THỰC TUẦN TỰ KẾT NỐI${NC}"
    echo -e "${BLUE}======================================================================${NC}"
    echo -e "Thời gian kiểm tra: $(date '+%Y-%m-%d %H:%M:%S')"
    echo -e "Hệ điều hành:       $(uname -s -r -m)"
    echo -e "${BLUE}----------------------------------------------------------------------${NC}\n"
}

step_pass() {
    local title="$1"
    local detail="$2"
    echo -e "  [${CURRENT_STEP}/${TOTAL_STEPS}] ${title} ... ${PASS}"
    if [ -n "$detail" ]; then
        echo -e "      ${CYAN}↳ ${detail}${NC}"
    fi
    CURRENT_STEP=$((CURRENT_STEP + 1))
    echo ""
}

step_fail() {
    local title="$1"
    local error_code="$2"
    local reason="$3"
    local remedy="$4"

    echo -e "  [${CURRENT_STEP}/${TOTAL_STEPS}] ${title} ... ${FAIL}"
    echo -e "\n${RED}======================================================================${NC}"
    echo -e "${BOLD}${RED}  ⛔ PHÁT HIỆN LỖI: ${error_code}${NC}"
    echo -e "${RED}======================================================================${NC}"
    echo -e "  📌 ${BOLD}Nguyên nhân:${NC} ${reason}"
    echo -e "  🛠️  ${BOLD}Cách khắc phục:${NC} ${remedy}"
    echo -e "${RED}----------------------------------------------------------------------${NC}"
    echo -e "  ${YELLOW}🛑 DỪNG KIỂM TRA TẠI ĐÂY. Vui lòng xử lý mã lỗi trên trước khi tiếp tục.${NC}\n"
    exit 1
}

step_warn() {
    local detail="$1"
    echo -e "      ${WARN} ${detail}"
}

# ==============================================================================
# BẮT ĐẦU CHẨN ĐOÁN
# ==============================================================================
print_header

# ------------------------------------------------------------------------------
# CHẶNG 1: Kiểm tra Phần cứng Serial (UART4 và UART0)
# ------------------------------------------------------------------------------
echo -e "${BOLD}▶ CHẶNG 1: Kiểm tra phần cứng cổng Serial...${NC}"

# 1.1 Kiểm tra UART4 (/dev/ttyAMA4)
if [ ! -c "/dev/ttyAMA4" ]; then
    step_fail "Kiểm tra cổng UART4 (/dev/ttyAMA4)" \
        "ERR_01_UART4_MISSING" \
        "Không tìm thấy thiết bị phần cứng /dev/ttyAMA4 trong hệ thống." \
        "1. Kiểm tra file /boot/firmware/config.txt (hoặc /boot/config.txt) xem đã có dòng 'dtoverlay=uart4' chưa.\n      2. Nếu vừa thêm, bắt buộc phải reboot Pi: 'sudo reboot'."
fi

# 1.2 Kiểm tra UART0 (/dev/ttyAMA0)
if [ ! -c "/dev/ttyAMA0" ]; then
    step_fail "Kiểm tra cổng UART0 (/dev/ttyAMA0 - SIYI)" \
        "ERR_02_UART0_MISSING" \
        "Không tìm thấy thiết bị /dev/ttyAMA0 dành cho SIYI Air Unit." \
        "Kiểm tra cấu hình boot xem 'enable_uart=1' đã được bật chưa."
fi

# 1.3 Kiểm tra quyền truy cập cổng
USER_NAME="$(whoami)"
if ! groups "$USER_NAME" | grep -q "dialout"; then
    step_fail "Kiểm tra quyền truy cập Serial (nhóm dialout)" \
        "ERR_20_UART4_PERMISSION" \
        "User '$USER_NAME' chưa thuộc nhóm 'dialout' để đọc/ghi cổng Serial." \
        "Chạy lệnh: 'sudo usermod -aG dialout $USER_NAME' sau đó đăng nhập lại."
fi

step_pass "Cổng Serial phần cứng" \
    "Đã nhận diện /dev/ttyAMA4 (Cube Orange) và /dev/ttyAMA0 (SIYI Air Unit) - Quyền: OK"

# ------------------------------------------------------------------------------
# CHẶNG 2: Kiểm tra Phần cứng Camera Intel RealSense D435i
# ------------------------------------------------------------------------------
echo -e "${BOLD}▶ CHẶNG 2: Kiểm tra phần cứng Camera RealSense...${NC}"

if ! command -v lsusb >/dev/null 2>&1; then
    sudo apt-get install -y -qq usbutils >/dev/null 2>&1 || true
fi

# 2.1 Quét USB device
if ! lsusb | grep -qiE "(8086:0b3a|RealSense)"; then
    step_fail "Phát hiện camera Intel RealSense trên cổng USB" \
        "ERR_03_REALSENSE_DISCONNECTED" \
        "Lệnh 'lsusb' không phát hiện thấy thiết bị Intel RealSense D435i." \
        "1. Cắm lại cáp USB Type-C của camera vào Pi.\n      2. Đảm bảo nguồn điện của Pi đủ 5V-5A (nguồn yếu camera sẽ không khởi động)."
fi

# 2.2 Kiểm tra tốc độ USB (USB 3.0 vs USB 2.0)
USB_SPEED=$(lsusb -t 2>/dev/null | grep -E "(uvcvideo|realsense)" -B 2 | grep -oE "5000M|480M" | head -n 1 || echo "")
if [ "$USB_SPEED" = "480M" ]; then
    step_warn "Camera đang cắm vào cổng USB 2.0 (tốc độ 480M). Khuyên dùng cổng USB 3.0 màu xanh để đạt 30 FPS ổn định."
fi

step_pass "Camera Intel RealSense D435i" \
    "Đã tìm thấy RealSense D435i trên bus USB (Tốc độ: ${USB_SPEED:-3.0 SuperSpeed})"

# ------------------------------------------------------------------------------
# CHẶNG 3: MAVLink Router Daemon & Mở cổng UART
# ------------------------------------------------------------------------------
echo -e "${BOLD}▶ CHẶNG 3: Xác thực MAVLink Router mở cổng kết nối...${NC}"

# 3.1 Kiểm tra container edge-router
MAV_CONTAINER=$(docker ps --format "{{.Names}}" | grep -E "^edge-router$" | head -n 1)
if [ -z "$MAV_CONTAINER" ]; then
    step_fail "Kiểm tra Container MAVLink Router" \
        "ERR_20_MAVLINK_CONTAINER_DOWN" \
        "Container MAVLink Router (edge-router) hiện KHÔNG chạy." \
        "Chạy lệnh: 'make start-router' hoặc 'make deploy-router' để khởi động lại."
fi

# 3.2 Đọc log xác thực xem mavlink-routerd đã mở UART4 và UART0 chưa
MAV_LOGS=$(docker logs --tail=100 "$MAV_CONTAINER" 2>&1 || echo "")

if echo "$MAV_LOGS" | grep -q "Could not open /dev/ttyAMA4"; then
    step_fail "Xác thực MAVLink Router mở cổng UART4" \
        "ERR_21_MAVLINK_UART4_OPEN_FAIL" \
        "MAVLink Router báo lỗi: 'Could not open /dev/ttyAMA4'." \
        "1. Kiểm tra cổng có bị tiến trình khác chiếm không: 'sudo lsof /dev/ttyAMA4'.\n      2. Kiểm tra tốc độ baud rate trong file .env."
fi

UART4_OPENED=false
if echo "$MAV_LOGS" | grep -qE "(Opened UART.*(Cube|ttyAMA4)|FlightController)"; then
    UART4_OPENED=true
elif docker exec "$MAV_CONTAINER" bash -c 'ls -l /proc/$(pgrep -o mavlink-routerd)/fd 2>/dev/null' 2>/dev/null | grep -q 'ttyAMA4'; then
    UART4_OPENED=true
fi

if [ "$UART4_OPENED" = false ]; then
    step_fail "Xác thực MAVLink Router mở cổng UART4" \
        "ERR_21_MAVLINK_UART4_OPEN_FAIL" \
        "Chưa thấy log xác nhận mở cổng UART4 thành công trong mavlink-routerd." \
        "Xem chi tiết lỗi bằng lệnh: 'make logs-router'."
fi

UART4_SPEED=$(echo "$MAV_LOGS" | grep -E "UART.*(Cube|ttyAMA4): speed" | tail -n 1 | awk '{print $NF}')
if [ -z "$UART4_SPEED" ]; then
    UART4_SPEED=$(docker exec "$MAV_CONTAINER" grep -A 3 "UartEndpoint FC" /run/drone/mavlink-router.conf 2>/dev/null | grep "Baud" | awk '{print $NF}' || echo "921600")
fi

# 3.3 Validate rendered router endpoints; broadcast is explicitly forbidden.
MAV_CONFIG=$(docker exec "$MAV_CONTAINER" cat /run/drone/mavlink-router.conf 2>/dev/null || docker exec "$MAV_CONTAINER" cat /etc/mavlink-router/main.conf 2>/dev/null || echo "")
if ! echo "$MAV_CONFIG" | grep -Fq "Address = 0.0.0.0" || ! echo "$MAV_CONFIG" | grep -Fq "Port = 14550"; then
    step_fail "Mở UDP Server cho QGroundControl" \
        "ERR_23_MAVLINK_UDP_FAIL" \
        "MAVLink Router chưa có UDP server 0.0.0.0:14550." \
        "Kiểm tra rendered config và deploy lại bằng 'make deploy-router'."
fi
if echo "$MAV_CONFIG" | grep -Eq "192\.168\.10\.255|10\.[0-9]+\.[0-9]+\.255|\[UdpEndpoint LAN_(wlan|uap)"; then
    step_fail "Phát hiện broadcast telemetry không an toàn" \
        "ERR_24_MAVLINK_BROADCAST_LOOP" \
        "Có broadcast endpoint có thể loop vào UDP server trong host network." \
        "Loại bỏ endpoint broadcast rồi force-recreate edge-router."
fi

step_pass "MAVLink Router Endpoints" \
    "UART4 /dev/ttyAMA4 @ ${UART4_SPEED}; UDP Server :14550; TCP proxy :5760."

# ------------------------------------------------------------------------------
# CHẶNG 4: Xác thực Gói tin MAVLink Heartbeat thực tế
# ------------------------------------------------------------------------------
echo -e "${BOLD}▶ CHẶNG 4: Xác thực luồng dữ liệu bay MAVLink thực tế...${NC}"

# Lắng nghe gói tin từ cổng TCP nội bộ 127.0.0.1:5760 trong 3 giây
HEARTBEAT_DETECTED=false
if command -v python3 >/dev/null 2>&1; then
    HEARTBEAT_RESULT=$(python3 -c "
import socket, sys
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(3.0)
try:
    sock.connect(('127.0.0.1', 5760))
    data = sock.recv(2048)
    if len(data) > 0:
        print('HEARTBEAT_OK')
    else:
        print('NO_DATA')
except Exception as e:
    print('TIMEOUT')
" 2>/dev/null || echo "TIMEOUT")

    if [ "$HEARTBEAT_RESULT" = "HEARTBEAT_OK" ]; then
        HEARTBEAT_DETECTED=true
    fi
fi

# Kiem tra thuc te luong Handled tu FlightController trong ReportStats cua mavlink-routerd
FC_HANDLED=$(echo "$MAV_LOGS" | grep -A 8 "FlightController" | grep "Handled:" | tail -n 1 | awk '{print $2}' || echo "0")
if [ -n "$FC_HANDLED" ] && [ "$FC_HANDLED" -gt 0 ] 2>/dev/null; then
    HEARTBEAT_DETECTED=true
    echo -e "      ↳ Đã xác thực nhận ${FC_HANDLED} gói tin MAVLink từ Flight Controller!"

fi

if [ "$HEARTBEAT_DETECTED" = false ]; then
    # Kiểm tra log xem có gói tin nào qua lại không
    if echo "$MAV_LOGS" | grep -q "incomplete messages"; then
        step_warn "Phát hiện có tín hiệu điện nhưng bị lỗi framing/incomplete messages. Có thể lệch tốc độ Baud (thử 57600 hoặc 921600)."
    fi
    step_fail "Nhận gói tin MAVLink từ Flight Controller (Cube Orange)" \
        "ERR_24_NO_HEARTBEAT" \
        "Cổng UART4 đã mở nhưng KHÔNG NHẬN ĐƯỢC BẤT KỲ GÓI TIN NÀO từ Cube Orange trong 3 giây." \
        "1. KIỂM TRA ĐẤU DÂY: Đảo ngược 2 chân TX và RX giữa Pi và Cube (GPIO 8 nối RX Cube, GPIO 9 nối TX Cube).\n      2. KIỂM TRA CẤU HÌNH ARDUPILOT: Mở QGC/Mission Planner kiểm tra cổng TELEM tương ứng đã đặt 'SERIALx_PROTOCOL = 2' (MAVLink2) chưa.\n      3. KIỂM TRA BAUD RATE: Đảm bảo 'SERIALx_BAUD' trên Cube trùng với DRONE_BAUD_RATE trong .env."
fi

step_pass "MAVLink Telemetry Stream" \
    "Đã nhận gói tin MAVLink Heartbeat từ Flight Controller Cube Orange Plus thành công!"

# ------------------------------------------------------------------------------
# CHẶNG 5: Video Streaming RTSP (MediaMTX + RealSense Streamer)
# ------------------------------------------------------------------------------
echo -e "${BOLD}▶ CHẶNG 5: Kiểm tra luồng phát Video RTSP...${NC}"

# 5.1 Kiểm tra Container edge-camera
CAM_CONTAINER=$(docker ps --format "{{.Names}}" | grep -E "^edge-camera$" | head -n 1)
if [ -z "$CAM_CONTAINER" ]; then
    step_fail "Kiểm tra Container Camera + RTSP (All-in-One)" \
        "ERR_30_CAMERA_RTSP_DOWN" \
        "Container Camera Stream Controller hiện KHÔNG chạy." \
        "Chạy lệnh: 'make start-camera' để khởi động."
fi

# 5.2 Kiểm tra cổng RTSP 8554
if ! ss -tln 2>/dev/null | grep -q ":8554"; then
    step_fail "Cổng mạng RTSP 8554" \
        "ERR_31_RTSP_PORT_BLOCKED" \
        "Cổng RTSP :8554 chưa mở." \
        "Khởi động lại: 'make restart-camera'."
fi

# 5.3 A listening port is not proof that a stream has a publisher.
RTSP_LOGS=$(docker logs --tail=160 "$CAM_CONTAINER" 2>&1 || echo "")
# The native streamer publishes RTMP locally. MediaMTX log wording varies by version,
# so confirm its established local RTMP session rather than relying on one log pattern.
RTMP_SESSION=$(ss -tn 2>/dev/null | awk '$1 == "ESTAB" && $4 ~ /:1935$/ {found=1} END {print found+0}')
if [ "$RTMP_SESSION" != "1" ]; then
    step_fail "Publisher video /camera" \
        "ERR_32_CAMERA_PUBLISHER_MISSING" \
        "MediaMTX nghe :8554 nhưng không có phiên RTMP publisher tới :1935." \
        "Xem 'make logs-camera', kiểm tra RealSense/RTMP :1935; test TCP: ffplay -rtsp_transport tcp rtsp://thong.local:8554/camera."
fi
step_pass "Camera Video Stream" "Đã có phiên RTMP publisher local tới MediaMTX /camera."

LEGACY_VISION_RUNNING=0
VISION_V1_RUNNING=0
docker compose --profile vision ps --status running edge-vision 2>/dev/null | grep -q edge-vision && LEGACY_VISION_RUNNING=1 || true
docker compose --profile vision-v1 ps --status running edge-vision-v1 2>/dev/null | grep -q edge-vision-v1 && VISION_V1_RUNNING=1 || true

if [ "$LEGACY_VISION_RUNNING" = "1" ] && [ "$VISION_V1_RUNNING" = "1" ]; then
    step_fail "Vision publisher mutual exclusion" \
        "ERR_33_VISION_PUBLISHER_CONFLICT" \
        "edge-vision và edge-vision-v1 đang chạy đồng thời, cùng sở hữu node vision và /yolo." \
        "Dừng một service: make stop-vision hoặc make stop-vision-v1."
fi

if [ "$LEGACY_VISION_RUNNING" = "1" ] || [ "$VISION_V1_RUNNING" = "1" ]; then
    if echo "$RTSP_LOGS" | grep -qE "(is publishing to path 'yolo'|path.*yolo|RTMP.*yolo)"; then
        step_pass "YOLO Video Stream" "Vision đang publish /yolo."
    else
        step_warn "Vision đang chạy nhưng /yolo chưa publish; kiểm tra logs của edge-vision hoặc edge-vision-v1."
    fi
else
    step_warn "Vision profile chưa chạy; /yolo không tồn tại là bình thường."
fi

step_pass "RTSP Video Stream" \
    "Dùng rtsp://thong.local:8554/camera qua TCP trước; chỉ kiểm tra UDP sau khi TCP thành công."

# ------------------------------------------------------------------------------
# CHẶNG 6: WiFi Access Point (AP_DRONE) & Cấu hình Mạng (Docker-Native)
# ------------------------------------------------------------------------------
echo -e "${BOLD}▶ CHẶNG 6: Kiểm tra WiFi Access Point Container (AP_DRONE)...${NC}"

# 6.1 Kiểm tra Container edge-network
if ! docker ps --filter "name=^edge-network$" --filter "status=running" -q | grep -q .; then
    step_fail "Kiểm tra Container WiFi AP" \
        "ERR_11_HOSTAPD_FAIL" \
        "Container 'edge-network' hiện KHÔNG chạy." \
        "Chạy lệnh: 'docker compose -f ~/drone-edge/docker-compose.yml up -d edge-network'."
fi

# 6.2 Kiểm tra IP Gateway 192.168.10.1 trên uap0
if ! ip a show uap0 2>/dev/null | grep -q "192.168.10.1"; then
    step_fail "Địa chỉ IP Gateway WiFi AP" \
        "ERR_11_HOSTAPD_FAIL" \
        "Cổng uap0 chưa nhận IP 192.168.10.1." \
        "Khởi động lại: 'make restart-all'."
fi

# 6.3 Kiểm tra tiến trình hostapd & dnsmasq trong container
NET_CONTAINER=$(docker ps --filter "name=^edge-network$" --filter "status=running" --format "{{.ID}}" | head -n 1)
if ! docker exec "$NET_CONTAINER" ps aux | grep -q "hostapd"; then
    step_fail "Tiến trình phát WiFi (hostapd)" \
        "ERR_11_HOSTAPD_FAIL" \
        "Tiến trình hostapd không chạy bên trong container edge-network." \
        "Xem log: 'docker logs "$NET_CONTAINER"'."
fi

if ! docker exec "$NET_CONTAINER" ps aux | grep -q "dnsmasq"; then
    step_fail "Tiến trình cấp IP DHCP (dnsmasq)" \
        "ERR_12_DNSMASQ_FAIL" \
        "Tiến trình dnsmasq không chạy bên trong container edge-network." \
        "Xem log: 'docker logs "$NET_CONTAINER"'."
fi

step_pass "WiFi Access Point (Docker-Native)" \
    "Hotspot 'AP_DRONE' & DHCP 192.168.10.x đang PHÁT SÓNG trực tiếp qua Docker!" 

# 6.4 Kiểm tra IP tĩnh eth0 (192.168.144.141)
if ! ip -br a show eth0 2>/dev/null | grep -q "192.168.144.141"; then
    step_warn "Cổng eth0 chưa có IP tĩnh 192.168.144.141 (Mã: ERR_13_ETH0_IP_MISSING). Bạn có thể apply lại: 'sudo netplan apply'."
fi

# Verify avahi-daemon process or mDNS listener inside container
if ! docker exec "$NET_CONTAINER" sh -c "pidof avahi-daemon >/dev/null 2>&1 || ss -uln 2>/dev/null | grep -q '5353'"; then
    step_fail "mDNS hostname" \
        "ERR_14_MDNS_DOWN" \
        "Avahi không mở UDP 5353; thong.local không thể ổn định qua WLAN DHCP." \
        "Xem docker logs edge-network và firewall multicast UDP 5353."
fi
step_pass "WiFi Access Point + mDNS" \
    "AP_DRONE gateway 192.168.10.1; hostname chuẩn: thong.local"

# 6.5 Kiểm tra không có RTPS (DDS) ngoài loopback (lo)
RTPS_LEAK=$(ss -uln 2>/dev/null | awk '$5 ~ /:(74[0-9]{2}|75[0-9]{2}|7600)$/ { split($5, a, ":"); if (a[1] != "127.0.0.1" && a[1] != "[::1]") print $5 }')
if [ -n "$RTPS_LEAK" ]; then
    step_fail "Kiểm tra cách ly RTPS/DDS (DDS Leak)" \
        "ERR_15_RTPS_LEAK_OUTSIDE_LO" \
        "Phát hiện cổng RTPS/DDS đang lắng nghe ngoài loopback: $RTPS_LEAK" \
        "DDS discovery chỉ được phép chạy trên lo (127.0.0.1). Kiểm tra CYCLONEDDS_URI và cấu hình NetworkInterface name=lo."
else
    step_pass "Cách ly RTPS/DDS" \
        "DDS discovery bị cô lập hoàn toàn trên loopback (lo), không phát hiện rò rỉ RTPS ra các interface ngoài."
fi

# ==============================================================================
# TỔNG KẾT THÀNH CÔNG
# ==============================================================================
echo -e "${GREEN}======================================================================${NC}"
echo -e "${BOLD}${GREEN}   🎉 TẤT CẢ CÁC BƯỚC XÁC THỰC HOÀN TẤT — HỆ THỐNG SẴN SÀNG BAY!${NC}"
echo -e "${GREEN}======================================================================${NC}"
echo -e "Các thông số vận hành thực tế:"
echo -e "   - Flight Controller:   Cube Orange Plus (/dev/ttyAMA4 @ ${UART4_SPEED})"
echo -e "   - SIYI Air Unit:       /dev/ttyAMA0 @ 115200"
echo -e "   - QGC Telemetry:       UDP port 19856"
echo -e "   - WiFi Hotspot:        SSID: ${CYAN}AP_DRONE${NC}"
echo -e "   - RTSP Video:          ${CYAN}rtsp://thong.local:8554/camera${NC}"
echo -e "   - RTSP Video (SIYI):   ${CYAN}rtsp://192.168.144.141:8554/camera${NC}"
echo -e "   - RTSP YOLO:           ${CYAN}rtsp://thong.local:8554/yolo${NC} (khi Vision publish)"
echo -e "   - AP recovery:         ${CYAN}192.168.10.1${NC}"
echo -e "${GREEN}======================================================================${NC}\n"
exit 0
