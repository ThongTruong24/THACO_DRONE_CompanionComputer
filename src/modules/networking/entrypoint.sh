#!/bin/bash
set -e

echo "======================================================"
echo "   🌐 DRONE NETWORKING CONTROLLER"
echo "   WiFi AP-STA Concurrency + Multi-Ethernet LAN"
echo "   ROS 2 Dynamic Network Node"
echo "======================================================"

# Kernel buffer optimization
sysctl -w net.core.rmem_max=4194304 >/dev/null 2>&1 || true
sysctl -w net.core.wmem_max=4194304 >/dev/null 2>&1 || true
sysctl -w net.core.rmem_default=1048576 >/dev/null 2>&1 || true
sysctl -w net.core.wmem_default=1048576 >/dev/null 2>&1 || true

WIFI_PARENT="${WIFI_PARENT:-wlan0}"
AP_IFACE="${AP_IFACE:-uap0}"
AP_IP="${AP_IP:-192.168.10.1}"
AP_SUBNET="${AP_SUBNET:-24}"
AP_COUNTRY="${AP_COUNTRY:-US}"
ETH0_SIYI_IP="${ETH0_SIYI_IP:-192.168.144.141}"
DNSMASQ_SRC="${DNSMASQ_SRC:-/app/config/dnsmasq.conf}"
[ -f "${DNSMASQ_SRC}" ] || DNSMASQ_SRC="/etc/dnsmasq.conf"
[ -f "${DNSMASQ_SRC}" ] || DNSMASQ_SRC="/app/dnsmasq.conf"

HOSTAPD_RUN="/tmp/hostapd.conf"
DNSMASQ_RUN="/tmp/dnsmasq.conf"

# Kill lingering daemons from previous run
killall -9 hostapd dnsmasq 2>/dev/null || true
rm -rf /var/run/wpa_supplicant/* /tmp/hostapd.conf /tmp/dnsmasq.conf 2>/dev/null || true

# Remove old uap0 virtual interface if leftover
if ip link show "${AP_IFACE}" >/dev/null 2>&1; then
    iw dev "${AP_IFACE}" del 2>/dev/null || true
fi

# 1. Ethernet eth0 Dual-IP: Keep DHCP / lab IP + add static SIYI IP on alias eth0:1
if ip link show eth0 >/dev/null 2>&1; then
    ip addr del "${ETH0_SIYI_IP}/24" dev eth0 2>/dev/null || true
    ip addr add "${ETH0_SIYI_IP}/24" dev eth0 label eth0:1 2>/dev/null || true
    echo "   ✓ eth0 configured with static IP ${ETH0_SIYI_IP}/24 (label eth0:1 for SIYI & RTSP)."
fi

# 2. Secondary LAN ports (eth1, eth2 for external USB-LAN / MAVLink Router)
if ip link show eth1 >/dev/null 2>&1; then
    ip addr flush dev eth1 2>/dev/null || true
    ip addr add 192.168.11.1/24 dev eth1 2>/dev/null || true
    ip link set eth1 up 2>/dev/null || true
    echo "   ✓ eth1 configured with 192.168.11.1/24"
fi

if ip link show eth2 >/dev/null 2>&1; then
    ip addr flush dev eth2 2>/dev/null || true
    ip addr add 192.168.12.1/24 dev eth2 2>/dev/null || true
    ip link set eth2 up 2>/dev/null || true
    echo "   ✓ eth2 configured with 192.168.12.1/24"
fi

# 3. Unblock WiFi rfkill
rfkill unblock wifi 2>/dev/null || true
ip link set "${WIFI_PARENT}" up 2>/dev/null || true

# 4. Create virtual AP interface
if ! ip link show "${AP_IFACE}" >/dev/null 2>&1; then
    iw dev "${WIFI_PARENT}" interface add "${AP_IFACE}" type __ap 2>/dev/null || true
fi

if ip link show "${AP_IFACE}" >/dev/null 2>&1; then
    ip addr flush dev "${AP_IFACE}" 2>/dev/null || true
    ip addr add "${AP_IP}/${AP_SUBNET}" dev "${AP_IFACE}" 2>/dev/null || true
    ip link set "${AP_IFACE}" up
    echo "   ✓ Interface ${AP_IFACE} created with IP ${AP_IP}/${AP_SUBNET}"
else
    echo "   [WARN] Could not create ${AP_IFACE}; running in degraded mode"
fi

# 5. Generate hostapd.conf cleanly via python (no sed!)
python3 -m networking.hotspot_config \
    --output "${HOSTAPD_RUN}" \
    --wlan-iface "${WIFI_PARENT}" \
    --ap-iface "${AP_IFACE}" \
    --country "${AP_COUNTRY}" || true

# 6. Start DHCP server (dnsmasq)
if [ -f "${DNSMASQ_SRC}" ]; then
    cp "${DNSMASQ_SRC}" "${DNSMASQ_RUN}"
    dnsmasq -C "${DNSMASQ_RUN}" -d &
    DNSMASQ_PID=$!
    echo "   ✓ dnsmasq started"
fi

# 7. Start hostapd in background
if [ -f "${HOSTAPD_RUN}" ] && ip link show "${AP_IFACE}" >/dev/null 2>&1; then
    hostapd -d "${HOSTAPD_RUN}" &
    HOSTAPD_PID=$!
    echo "   ✓ hostapd started"
fi

# Cleanup on SIGTERM/SIGINT
cleanup() {
    echo "🛑 Stopping networking services..."
    kill -9 "${DNSMASQ_PID}" 2>/dev/null || true
    kill -9 "${HOSTAPD_PID}" 2>/dev/null || true
    killall -9 hostapd dnsmasq 2>/dev/null || true
    iw dev "${AP_IFACE}" del 2>/dev/null || true
    exit 0
}
trap cleanup SIGTERM SIGINT

# 8. Source ROS 2 environment and run networking_node
[ -f /opt/ros/jazzy/setup.bash ] && . /opt/ros/jazzy/setup.bash
[ -f /opt/cc/setup.bash ] && . /opt/cc/setup.bash
[ -f /app/install/setup.bash ] && . /app/install/setup.bash

exec ros2 run networking networking_node "$@"
