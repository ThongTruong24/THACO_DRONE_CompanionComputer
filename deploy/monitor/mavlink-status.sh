#!/usr/bin/env bash
# ==============================================================================
# deploy/monitor/mavlink-status.sh - Giám sát MAVLink Router (Cổng & Dữ liệu mỗi kênh)
# Chế độ:
#   1: Xem danh sách cổng, endpoints, sockets đang mở và thiết bị kết nối
#   2: Xem dữ liệu gửi/nhận thời gian thực từng kênh (ReportStats)
# ==============================================================================

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

show_ports() {
    echo -e "${CYAN}${BOLD}======================================================================${NC}"
    echo -e "${CYAN}${BOLD}     CHẾ ĐỘ 1: DANH SÁCH CỔNG & TRẠNG THÁI ENDPOINTS MAVLINK ROUTER  ${NC}"
    echo -e "${CYAN}${BOLD}======================================================================${NC}"
    echo ""

    echo -e "${YELLOW}${BOLD}[1] CỔNG PHẦN CỨNG ĐẦU VÀO / RA (SERIAL UART):${NC}"
    echo -e "  • ${BOLD}Flight Controller${NC}: /dev/ttyAMA4 (UART4) | Baud: ${GREEN}921600${NC} (Cube Orange Plus)"
    if [ -e /dev/ttyAMA4 ]; then
        echo -e "    -> Trạng thái: ${GREEN}✓ Sẵn sàng (/dev/ttyAMA4 tồn tại)${NC}"
    else
        echo -e "    -> Trạng thái: ${RED}✗ Không tìm thấy /dev/ttyAMA4${NC}"
    fi

    echo -e "  • ${BOLD}SIYI Air Unit${NC}:     /dev/ttyAMA0 (UART0) | Baud: ${GREEN}115200${NC} (Telemetry Serial)"
    if [ -e /dev/ttyAMA0 ]; then
        echo -e "    -> Trạng thái: ${GREEN}✓ Sẵn sàng (/dev/ttyAMA0 tồn tại)${NC}"
    else
        echo -e "    -> Trạng thái: ${RED}✗ Không tìm thấy /dev/ttyAMA0${NC}"
    fi
    echo ""

    echo -e "${YELLOW}${BOLD}[2] CÁC ĐẦU RA MẠNG (CONFIGURED NETWORK ENDPOINTS):${NC}"
    printf "  %-24s %-8s %-24s %-12s %s\n" "Tên Endpoint" "Giao thức" "Địa chỉ:Port" "Chế độ" "Mục tiêu / Thiết bị"
    echo -e "  ----------------------------------------------------------------------------------------------------"
    printf "  %-24s %-8s %-24s %-12s %s\n" "QGC_UDP_Server" "UDP" "0.0.0.0:14550" "Server" "Active GCS clients on any Pi interface"
    printf "  %-24s %-8s %-24s %-12s %s\n" "MAVROS_Companion" "UDP" "127.0.0.1:14541" "Server" "Dedicated MAVROS bridge route"
    
    # Hiển thị các endpoint LAN được tự động phát hiện
    if docker exec mavlink-router-controller grep -q "\[UdpEndpoint LAN_" /etc/mavlink-router/main.conf 2>/dev/null; then
        docker exec mavlink-router-controller awk '
            /\[UdpEndpoint LAN_/ { gsub(/[\[\]]/, "", $2); name=$2 }
            name && /Address =/ { addr=$3 }
            name && /Port =/ { port=$3 }
            name && /Mode =/ { mode=$3 }
            name && addr && port && mode {
                printf "  %-24s %-8s %-24s %-12s %s\n", name, "UDP", addr ":" port, mode, "Mạng LAN tự động phát hiện";
                name=""; addr=""; port=""; mode=""
            }
        ' /etc/mavlink-router/main.conf
    fi

    printf "  %-24s %-8s %-24s %-12s %s\n" "Internal_Companion" "UDP" "127.0.0.1:14540" "Server" "TCP proxy / local tools"
    printf "  %-24s %-8s %-24s %-12s %s\n" "TCP_Server" "TCP" "0.0.0.0:5760" "Server" "TCP MAVLink Server"
    echo ""

    echo -e "${YELLOW}${BOLD}[3] SOCKET MẠNG ĐANG MỞ THỰC TẾ (SOCKET PORTS):${NC}"
    ss -ulpn 'sport = :14550 or sport = :14540 or sport = :14541' 2>/dev/null | awk 'NR>1 {print "  • UDP " $4}' || true
    ss -tlpn 'sport = :5760' 2>/dev/null | awk 'NR>1 {print "  • TCP " $4}' || true
    echo ""

    echo -e "${YELLOW}${BOLD}[4] TRẠNG THÁI IP & THIẾT BỊ ĐANG ONLINE (ACTIVE IP & CLIENTS):${NC}"
    echo -e "  • Cổng Ethernet (${CYAN}eth0${NC}):"
    ip -o -4 addr show eth0 2>/dev/null | while read line; do
        ip_addr=$(echo "$line" | awk '{print $4}')
        label=$(echo "$line" | awk '{print $2}')
        if [[ "$ip_addr" =~ 192\.168\.144 ]]; then
            echo -e "    -> ${GREEN}$ip_addr${NC} (IP tĩnh SIYI / RTSP Video - $label)"
        else
            echo -e "    -> ${GREEN}$ip_addr${NC} (Mạng dây LAN phòng Lab - $label)"
        fi
    done
    
    echo -e "  • Mạng WiFi AP (${CYAN}uap0 - 192.168.10.x${NC}):"
    wifi_clients=$(arp -an -i uap0 2>/dev/null | grep -v incomplete || true)
    if [ -n "$wifi_clients" ]; then
        echo "$wifi_clients" | while read line; do
            ip=$(echo "$line" | grep -oP '\(\K[^\)]+')
            mac=$(echo "$line" | awk '{print $4}')
            echo -e "    -> Thiết bị: ${GREEN}$ip (MAC: $mac)${NC}"
        done
    else
        echo -e "    -> ${YELLOW}(Chưa có client kết nối WiFi AP_DRONE)${NC}"
    fi
    echo ""
    echo -e "${CYAN}======================================================================${NC}"
}

show_stats() {
    echo -e "${GREEN}${BOLD}======================================================================${NC}"
    echo -e "${GREEN}${BOLD}     CHẾ ĐỘ 2: THEO DÕI DỮ LIỆU GỬI/NHẬN MỖI KÊNH (REAL-TIME STATS)   ${NC}"
    echo -e "${GREEN}${BOLD}======================================================================${NC}"
    echo -e "${YELLOW}Đang theo dõi lưu lượng MAVLink trực tiếp (Nhấn Ctrl+C để thoát)...${NC}\n"
    docker compose -f ~/drone-edge/docker-compose.yml logs -f --tail=60 mavlink-router-controller
}

MODE="${1:-}"

if [ -z "$MODE" ]; then
    echo -e "${BOLD}Lựa chọn chế độ xem MAVLink Router:${NC}"
    echo -e "  ${CYAN}[1]${NC} Xem danh sách cổng & trạng thái kết nối (Endpoints / Ports)"
    echo -e "  ${CYAN}[2]${NC} Xem dữ liệu gửi/nhận mỗi kênh thời gian thực (Traffic Stats)"
    read -t 5 -p "Chọn chế độ (1 hoặc 2) [Mặc định: 2]: " choice || true
    echo ""
    choice="${choice:-2}"
    if [ "$choice" = "1" ]; then
        show_ports
    else
        show_stats
    fi
elif [ "$MODE" = "1" ] || [ "$MODE" = "ports" ] || [ "$MODE" = "port" ]; then
    show_ports
elif [ "$MODE" = "2" ] || [ "$MODE" = "stats" ] || [ "$MODE" = "data" ]; then
    show_stats
else
    echo "Tham số không hợp lệ: $MODE (dùng 1/ports hoặc 2/stats)"
    exit 1
fi
