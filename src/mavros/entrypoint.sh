#!/usr/bin/env bash
set -Eeo pipefail

: "${MAVROS_FCU_URL:=udp://:14542@127.0.0.1:14541}"
: "${ROS_DOMAIN_ID:=0}"

case "$MAVROS_FCU_URL" in
  udp://:14542@127.0.0.1:14541) ;;
  *)
    echo "[FATAL] MAVROS_FCU_URL must use udp://:14542@127.0.0.1:14541" >&2
    exit 20
    ;;
esac

source /opt/ros/jazzy/setup.bash
source /opt/cc_ws/setup.bash

# Preload agridrone plugin to intercept MAVLink v2 CRC validation for THACO custom messages
export LD_PRELOAD=/opt/cc_ws/lib/libagridrone_plugins.so

echo "[mavros] ROS_DOMAIN_ID=${ROS_DOMAIN_ID}"
echo "[mavros] FCU URL=${MAVROS_FCU_URL}"
echo "[mavros] CC & QGC telemetry broadcaster enabled; bidirectional Ras <-> GCS active."

/usr/local/bin/mavros-telemetry-demo &
telemetry_pid=$!

ros2 run mavros mavros_node --ros-args \
  -r __node:=drone_mavros \
  -p fcu_url:="$MAVROS_FCU_URL" \
  -p tgt_system:=1 \
  -p tgt_component:=1 &
mavros_pid=$!

if [ -x /opt/cc_ws/lib/cc_telemetry/cc_telemetry_node ]; then
  /opt/cc_ws/lib/cc_telemetry/cc_telemetry_node &
elif [ -x /opt/cc_ws/lib/cc_telemetry_node ]; then
  /opt/cc_ws/lib/cc_telemetry_node &
else
  ros2 run cc_telemetry cc_telemetry_node &
fi
cc_pid=$!

cleanup() {
  echo "[mavros] Stopping MAVROS and telemetry observer..."
  kill -TERM "$mavros_pid" "$telemetry_pid" "$cc_pid" 2>/dev/null || true
  wait "$mavros_pid" 2>/dev/null || true
  wait "$telemetry_pid" 2>/dev/null || true
  wait "$cc_pid" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

echo "[mavros] Telemetry broadcaster started (pid=${cc_pid})."
wait -n "$mavros_pid" "$cc_pid"
