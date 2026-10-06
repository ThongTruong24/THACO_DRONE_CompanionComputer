#!/bin/bash
# Runs inside edge-vision (via /ros_entrypoint.sh, which already sourced ROS 2 and /opt/cc).
# "$@" is forwarded to the node, e.g. --ros-args --params-file /app/config/vision.yaml
set -e

if [ -z "$CAMERA_MODEL" ] && [ -f /app/config/vision.yaml ]; then
    CAMERA_MODEL=$(grep -E '^[[:space:]]*camera_model:' /app/config/vision.yaml | awk '{print $2}' | tr -d '"\r\n')
fi
CAMERA_MODEL="${CAMERA_MODEL:-realsense_d435i}"

CAMERA_PARAM_ARGS=()
if [ -f "/app/config/cameras/${CAMERA_MODEL}.yaml" ]; then
    echo "[vision] Nạp hồ sơ camera: /app/config/cameras/${CAMERA_MODEL}.yaml"
    CAMERA_PARAM_ARGS=("--params-file" "/app/config/cameras/${CAMERA_MODEL}.yaml")
fi

if command -v ros2 >/dev/null 2>&1; then
    exec ros2 run vision vision_node "${CAMERA_PARAM_ARGS[@]}" "$@"
else
    echo "[vision] ros2 binary not in container, fallback to standalone yolo_streamer..."
    export VISION_CONFIG="${VISION_CONFIG:-/app/config/drone.yaml}"
    exec python3 /app/scripts/yolo_streamer.py
fi
