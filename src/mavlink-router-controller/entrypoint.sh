#!/bin/bash
# entrypoint.sh - Khoi dong container MAVLink Router
# 1. Kiem tra phan cung va thay the bien moi truong
# 2. Tu dong enable SIYI UART neu ton tai phan cung
# 3. Khoi dong mavlink-routerd

set -Eeuo pipefail

. /tmp/drone_log.sh 2>/dev/null || true

DEFAULT_SERIAL="${DRONE_SERIAL_PORT:-/dev/ttyAMA4}"
DEFAULT_BAUD="${DRONE_BAUD_RATE:-921600}"
SIYI_SERIAL="${SIYI_SERIAL_PORT:-/dev/ttyAMA0}"
SIYI_BAUD="${SIYI_BAUD:-115200}"
SIYI_ENABLED="${SIYI_ENABLED:-true}"
GCS_IP="${GCS_IP:-}"

echo "========================================"
echo "  Drone Edge - MAVLink Router"
echo "  $(date '+%Y-%m-%d %H:%M:%S')"
echo "========================================"
echo "  Flight Controller : ${DEFAULT_SERIAL} @ ${DEFAULT_BAUD}"
echo "  SIYI Telemetry    : ${SIYI_SERIAL} @ ${SIYI_BAUD} (enabled: ${SIYI_ENABLED})"
echo "========================================"

TEMPLATE="/etc/mavlink-router/main.conf.template"
CONFIG="/etc/mavlink-router/main.conf"

mkdir -p /etc/mavlink-router /var/log/mavlink-router

# 1. Base Configuration from template (thay the bien moi truong FC)
sed -e "s|\${DRONE_SERIAL_PORT}|${DEFAULT_SERIAL}|g" \
    -e "s|\${DRONE_BAUD_RATE}|${DEFAULT_BAUD}|g" \
    "${TEMPLATE}" > "${CONFIG}"

# 2. Add SIYI UART Endpoint if device exists and is enabled
if [ "${SIYI_ENABLED}" = "true" ] || [ "${SIYI_ENABLED}" = "1" ]; then
    if [ -c "${SIYI_SERIAL}" ]; then
        echo "[entrypoint] Adding SIYI UART Endpoint: ${SIYI_SERIAL} @ ${SIYI_BAUD}"
        cat <<EOF >> "${CONFIG}"

# ============================================================
# SIYI Air Unit Telemetry (Hardware Detected: ${SIYI_SERIAL})
# ============================================================
[UartEndpoint SIYI_Serial]
Device = ${SIYI_SERIAL}
Baud = ${SIYI_BAUD}
FlowControl = false
EOF
    else
        echo "[entrypoint] Notice: SIYI port ${SIYI_SERIAL} absent - skipping SIYI endpoint (no crash)."
    fi
fi

# 3. Optional Direct GCS Unicast fallback
if [ -n "${GCS_IP}" ]; then
    echo "[entrypoint] Adding Direct GCS Unicast: ${GCS_IP}:14550"
    cat <<EOF >> "${CONFIG}"

[UdpEndpoint GCS_Direct]
Mode = Normal
Address = ${GCS_IP}
Port = 14550
EOF
fi

echo "[entrypoint] Generated /etc/mavlink-router/main.conf successfully:"
cat "${CONFIG}"

echo ""
echo "[entrypoint] Starting mavlink-routerd..."
exec mavlink-routerd -c "${CONFIG}"
