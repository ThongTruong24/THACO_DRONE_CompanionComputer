#!/bin/bash
# Runs inside edge-vision-v1 after /ros_entrypoint.sh has sourced ROS 2 and /opt/cc.
set -e

if [ -z "$CAMERA_MODEL" ] && [ -f /app/config/vision_v1.yaml ]; then
    CAMERA_MODEL=$(grep -E '^[[:space:]]*camera_model:' /app/config/vision_v1.yaml | awk '{print $2}' | tr -d '"\r\n')
fi
CAMERA_MODEL="${CAMERA_MODEL:-realsense_d435i}"

CAMERA_PARAM_ARGS=()
if [ -f "/app/config/cameras/${CAMERA_MODEL}.yaml" ]; then
    echo "[vision_v1] Nạp hồ sơ camera: /app/config/cameras/${CAMERA_MODEL}.yaml"
    CAMERA_PARAM_ARGS=("--params-file" "/app/config/cameras/${CAMERA_MODEL}.yaml")
fi

exec ros2 run vision_v1 vision_v1_node "${CAMERA_PARAM_ARGS[@]}" "$@"

