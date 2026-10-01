#!/bin/bash
set -e

echo "======================================================"
echo "   🌐 DRONE NETWORKING CONTROLLER (ALL-IN-ONE)"
echo "   WiFi AP-STA Concurrency + Multi-Ethernet DHCP LAN"
echo "   SOLID Dynamic WiFi Architecture & Hot-Reload"
echo "======================================================"

# 0. Dọn dẹp trạng thái mạng cũ & tối ưu Kernel Buffer
echo "▶ [0/7] Dọn dẹp trạng thái mạng cũ & tối ưu Kernel Buffer..."
sysctl -w net.core.rmem_max=4194304 >/dev/null 2>&1 || true
sysctl -w net.core.wmem_max=4194304 >/dev/null 2>&1 || true
sysctl -w net.core.rmem_default=1048576 >/dev/null 2>&1 || true
sysctl -w net.core.wmem_default=1048576 >/dev/null 2>&1 || true

WIFI_PARENT="${WIFI_PARENT:-wlan0}"
AP_IFACE="${AP_IFACE:-uap0}"
AP_IP="${AP_IP:-192.168.10.1}"
AP_SUBNET="${AP_SUBNET:-24}"
HOSTAPD_SRC="${HOSTAPD_SRC:-/etc/hostapd/hostapd.conf}"
DNSMASQ_SRC="${DNSMASQ_SRC:-/etc/dnsmasq.d/drone.conf}"
CONFIG_JSON="/app/config/wifi.json"

# Auto-provision python3 nếu image chưa có
command -v python3 >/dev/null 2>&1 || apk add --no-cache python3 2>/dev/null || true

# Đọc cấu hình từ Data Contract JSON (Ưu tiên cao nhất) hoặc fallback về biến môi trường
if [ -f "${CONFIG_JSON}" ] && command -v python3 >/dev/null 2>&1; then
    echo "▶ Nạp cấu hình từ Data Contract: ${CONFIG_JSON}..."
    WIFI_SSID=$(python3 -c "import json; print(json.load(open('${CONFIG_JSON}')).get('client', {}).get('ssid', ''))" 2>/dev/null || echo "")
    WIFI_PSK=$(python3 -c "import json; print(json.load(open('${CONFIG_JSON}')).get('client', {}).get('password', ''))" 2>/dev/null || echo "")
    AP_SSID=$(python3 -c "import json; print(json.load(open('${CONFIG_JSON}')).get('hotspot', {}).get('ssid', ''))" 2>/dev/null || echo "")
    AP_PASS=$(python3 -c "import json; print(json.load(open('${CONFIG_JSON}')).get('hotspot', {}).get('password', ''))" 2>/dev/null || echo "")
fi

WIFI_SSID="${WIFI_SSID:-${WIFI_CLIENT_SSID:-}}"
WIFI_PSK="${WIFI_PSK:-${WIFI_CLIENT_PASSWORD:-}}"
WIFI_KEY_MGMT="${WIFI_KEY_MGMT:-WPA-PSK}"

AP_SSID="${AP_SSID:-${WIFI_AP_SSID:-AP_DRONE}}"
AP_PASS="${AP_PASS:-${WIFI_AP_PASSWORD:-12345678}}"

# Tắt daemons cũ nếu còn sót lại
# This is host networking: never terminate wlan0 DHCP or another Avahi instance.
killall -9 hostapd dnsmasq 2>/dev/null || true
rm -rf /var/run/wpa_supplicant/* /tmp/hostapd.conf /tmp/dnsmasq.conf /tmp/wpa_client.conf 2>/dev/null || true

# Xóa interface ảo uap0 cũ nếu còn tồn dư từ container trước
if ip link show "${AP_IFACE}" >/dev/null 2>&1; then
    iw dev "${AP_IFACE}" del 2>/dev/null || true
fi

# 1. Cấu hình Cáp Mạng Ethernet eth0 (Dual-IP: Giữ DHCP Lab + Thêm IP tĩnh SIYI 192.168.144.141)
echo "▶ [1/7] Kiểm tra cấu hình cổng Ethernet eth0..."
if ip link show eth0 >/dev/null 2>&1; then
    ip addr del 192.168.144.141/24 dev eth0 2>/dev/null || true
    ip addr add 192.168.144.141/24 dev eth0 label eth0:1 2>/dev/null || true
    echo "   ✓ eth0 đã gán IP tĩnh 192.168.144.141/24 (nhãn alias eth0:1 cho SIYI & RTSP)."
fi

# 2. Quét và cấu hình các cổng Ethernet LAN phụ (eth1, eth2... cắm USB-LAN ngoài cho MAVLink Router)
echo "▶ [2/7] Quét các cổng Ethernet LAN phụ cho MAVLink Router..."
if ip link show eth1 >/dev/null 2>&1; then
    echo "   ✓ Phát hiện cổng eth1! Gán Gateway 192.168.11.1/24 và bật interface..."
    ip addr flush dev eth1 2>/dev/null || true
    ip addr add 192.168.11.1/24 dev eth1 2>/dev/null || true
    ip link set eth1 up 2>/dev/null || true
fi

if ip link show eth2 >/dev/null 2>&1; then
    echo "   ✓ Phát hiện cổng eth2! Gán Gateway 192.168.12.1/24 và bật interface..."
    ip addr flush dev eth2 2>/dev/null || true
    ip addr add 192.168.12.1/24 dev eth2 2>/dev/null || true
    ip link set eth2 up 2>/dev/null || true
fi

# 3. Mở khóa rfkill WiFi
echo "▶ [3/7] Mở khóa rfkill WiFi..."
rfkill unblock wifi 2>/dev/null || true
ip link set "${WIFI_PARENT}" up 2>/dev/null || true

# 4. Read the station state only. Netplan/networkd owns wlan0 and DHCP.
echo "[4/7] Reading WiFi station state from ${WIFI_PARENT} (managed by netplan)..."
WIFI_IP=$(ip -o -4 addr show dev "${WIFI_PARENT}" scope global 2>/dev/null | awk 'NR == 1 {print $4}')
if [ -n "${WIFI_IP}" ]; then
    echo "   WiFi client ready: ${WIFI_PARENT} has ${WIFI_IP}"
else
    echo "   WARN: ${WIFI_PARENT} has no DHCP IPv4 address; AP will still start."
fi

# 5. Phân tích tần số & Tự động thích ứng Băng tần / Kênh cho Hotspot
echo "▶ [5/7] Tự động thích ứng Kênh & Băng tần cho Hotspot..."
FREQ=$(iw dev "${WIFI_PARENT}" link 2>/dev/null | awk '/freq:/ {print int($2); exit}')

AP_HW_MODE="g"
AP_CHANNEL="6"
AP_IS_5G=false

if [ -n "${FREQ}" ]; then
    if [ "${FREQ}" -ge 5000 ]; then
        AP_CHANNEL=$(( (FREQ - 5000) / 5 ))
        AP_HW_MODE="a"
        AP_IS_5G=true
        echo "   ⚡ TỰ ĐỘNG ĐỒNG BỘ: WiFi Client đang chạy 5 GHz (Tần số: ${FREQ} MHz -> Kênh: ${AP_CHANNEL})"
    elif [ "${FREQ}" -ge 2400 ]; then
        if [ "${FREQ}" -eq 2484 ]; then
            AP_CHANNEL="14"
        else
            AP_CHANNEL=$(( (FREQ - 2407) / 5 ))
        fi
        AP_HW_MODE="g"
        AP_IS_5G=false
        echo "   ⚡ TỰ ĐỘNG ĐỒNG BỘ: WiFi Client đang chạy 2.4 GHz (Tần số: ${FREQ} MHz -> Kênh: ${AP_CHANNEL})"
    fi
else
    echo "   ⚠️ Không phát hiện liên kết WiFi ngoài (Chế độ bay ngoài trời / Mất sóng Lab)"
    echo "   ✓ Fallback an toàn: Phát AP độc lập ở 2.4 GHz (Kênh 6)"
    AP_HW_MODE="g"
    AP_CHANNEL="6"
    AP_IS_5G=false
fi

# Tạo interface ảo uap0
if ! ip link show "${AP_IFACE}" >/dev/null 2>&1; then
    iw dev "${WIFI_PARENT}" interface add "${AP_IFACE}" type __ap 2>/dev/null || true
fi

if ! ip link show "${AP_IFACE}" >/dev/null 2>&1; then
    echo "[FATAL] Cannot create ${AP_IFACE}; refusing to overwrite ${WIFI_PARENT} DHCP address."
    exit 12
fi

ip addr flush dev "${AP_IFACE}" 2>/dev/null || true
ip addr add "${AP_IP}/${AP_SUBNET}" dev "${AP_IFACE}" 2>/dev/null || true
ip link set "${AP_IFACE}" up

# Sinh file cấu hình hostapd.conf động
HOSTAPD_RUN="/tmp/hostapd.conf"
DNSMASQ_RUN="/tmp/dnsmasq.conf"
cp "${DNSMASQ_SRC}" "${DNSMASQ_RUN}"

cat > "${HOSTAPD_RUN}" << HTEOF
interface=${AP_IFACE}
driver=nl80211
ssid=${AP_SSID}
hw_mode=${AP_HW_MODE}
channel=${AP_CHANNEL}
wmm_enabled=1
auth_algs=1
ignore_broadcast_ssid=0

# WPA2-PSK Security
wpa=2
wpa_passphrase=${AP_PASS}
wpa_key_mgmt=WPA-PSK
wpa_pairwise=CCMP
rsn_pairwise=CCMP
HTEOF

if [ "${AP_IS_5G}" = "true" ]; then
    cat >> "${HOSTAPD_RUN}" << HTEOF5G
country_code=US
ieee80211d=1
ieee80211n=1
ieee80211ac=1
HTEOF5G
else
    cat >> "${HOSTAPD_RUN}" << HTEOF2G
ieee80211n=1
HTEOF2G
fi

# 6. Khởi động DHCP Server (dnsmasq), mDNS (Avahi) và hostapd
echo "▶ [6/7] Khởi động DHCP Server (dnsmasq), mDNS & WiFi AP..."
killall -9 dnsmasq 2>/dev/null || true
dnsmasq -C "${DNSMASQ_RUN}" -d &
DNSMASQ_PID=$!
sleep 1
if kill -0 "${DNSMASQ_PID}" 2>/dev/null; then
    echo "   ✓ dnsmasq DHCP/DNS listening on ${AP_IP}:53"
else
    echo "   [WARN] dnsmasq exited unexpectedly. Check port 53 or interface status."
fi

# Cấu hình và khởi động mDNS Daemon (Avahi)
MDNS_NAME="${MDNS_HOSTNAME:-drone}"
if ss -uln 2>/dev/null | grep -qE '[:.]5353[[:space:]]'; then
    echo "   [Notice] Host mDNS (avahi-daemon) is already active on UDP 5353."
    if command -v avahi-publish >/dev/null 2>&1; then
        avahi-publish -a -R "${MDNS_NAME}.local" "${AP_IP}" &
        AVAHI_PID=$!
    fi
else
    mkdir -p /etc/avahi
    cat > /etc/avahi/avahi-daemon.conf << AEOF
[server]
host-name=${MDNS_NAME}
use-ipv4=yes
use-ipv6=no
check-response-ttl=no
use-iff-running=yes
enable-dbus=no

[publish]
publish-addresses=yes
publish-hinfo=no
publish-workstation=no
publish-domain=yes
AEOF

    # Zero hardcoded static hosts - Avahi automatically registers all active interfaces (wlan0, eth0, uap0)
    rm -f /etc/avahi/hosts

    rm -rf /var/run/avahi-daemon/pid /run/avahi-daemon/pid
    avahi-daemon --no-drop-root &
    AVAHI_PID=$!
    sleep 1
    if kill -0 "${AVAHI_PID}" 2>/dev/null; then
        echo "   mDNS published: ${MDNS_NAME}.local on active wlan0/uap0 addresses"
    fi
fi

hostapd -d "${HOSTAPD_RUN}" &
HOSTAPD_PID=$!

echo "   WiFi Client: SSID=${WIFI_SSID} | IP=${WIFI_IP:-N/A}"
echo "   WiFi AP    : SSID=${AP_SSID} | Band=${AP_HW_MODE} (Kênh ${AP_CHANNEL}) | Gateway=${AP_IP}"
echo "   mDNS Domain: ${MDNS_NAME}.local (Tự động phân giải IP mọi dải mạng)"
echo "------------------------------------------------------"

# 7. Daemon giám sát Hot-Reload tự động (SOLID Dynamic Watcher)
echo "▶ [7/7] Kích hoạt Hot-Reload Watcher cho Data Contract (${CONFIG_JSON})..."
watch_wifi_contract() {
    local last_mtime=""
    if [ -f "${CONFIG_JSON}" ]; then
        last_mtime=$(stat -c %Y "${CONFIG_JSON}" 2>/dev/null || stat -f %m "${CONFIG_JSON}" 2>/dev/null || echo "")
    fi

    while true; do
        sleep 3
        if [ -f "${CONFIG_JSON}" ]; then
            local curr_mtime=$(stat -c %Y "${CONFIG_JSON}" 2>/dev/null || stat -f %m "${CONFIG_JSON}" 2>/dev/null || echo "")
            if [ -n "${last_mtime}" ] && [ "${curr_mtime}" != "${last_mtime}" ]; then
                echo "⚡ [Hot-Reload] Phát hiện thay đổi trong ${CONFIG_JSON}! Đang nạp lại cấu hình mạng..."
                last_mtime="${curr_mtime}"

                local NEW_AP_SSID=$(python3 -c "import json; print(json.load(open('${CONFIG_JSON}')).get('hotspot', {}).get('ssid', ''))" 2>/dev/null || echo "")
                local NEW_AP_PASS=$(python3 -c "import json; print(json.load(open('${CONFIG_JSON}')).get('hotspot', {}).get('password', ''))" 2>/dev/null || echo "")
                local NEW_STA_SSID=$(python3 -c "import json; print(json.load(open('${CONFIG_JSON}')).get('client', {}).get('ssid', ''))" 2>/dev/null || echo "")
                local NEW_STA_PASS=$(python3 -c "import json; print(json.load(open('${CONFIG_JSON}')).get('client', {}).get('password', ''))" 2>/dev/null || echo "")

                # 1. Cập nhật Hotspot nếu thay đổi
                if [ -n "${NEW_AP_SSID}" ] && [ -n "${NEW_AP_PASS}" ] && ([ "${NEW_AP_SSID}" != "${AP_SSID}" ] || [ "${NEW_AP_PASS}" != "${AP_PASS}" ]); then
                    echo "   ↳ Cập nhật Hotspot mới: SSID='${NEW_AP_SSID}'..."
                    AP_SSID="${NEW_AP_SSID}"
                    AP_PASS="${NEW_AP_PASS}"
                    sed -i "s/^ssid=.*/ssid=${AP_SSID}/" "${HOSTAPD_RUN}"
                    sed -i "s/^wpa_passphrase=.*/wpa_passphrase=${AP_PASS}/" "${HOSTAPD_RUN}"
                    killall -9 hostapd 2>/dev/null || true
                    sleep 1
                    hostapd -d "${HOSTAPD_RUN}" &
                    HOSTAPD_PID=$!
                    echo "   ✓ Hotspot đã nạp mật khẩu mới thành công!"
                fi

                # STA credentials are owned by netplan/networkd
                if [ -n "${NEW_STA_SSID}" ] && [ "${NEW_STA_SSID}" != "${WIFI_SSID}" ]; then
                    echo "   WARN: WiFi client settings changed; update netplan explicitly, then run make apply-netplan."
                fi
            fi
        fi
    done
}

watch_wifi_contract &
WATCHER_PID=$!

cleanup() {
    echo "🛑 Đang tắt Networking services & dọn dẹp interface ảo..."
    kill -9 ${WATCHER_PID} 2>/dev/null || true
    kill -9 ${DNSMASQ_PID} 2>/dev/null || true
    kill -9 ${AVAHI_PID} 2>/dev/null || true
    killall -9 hostapd dnsmasq 2>/dev/null || true
    iw dev "${AP_IFACE}" del 2>/dev/null || true
    exit 0
}

trap cleanup SIGTERM SIGINT

wait ${HOSTAPD_PID}
