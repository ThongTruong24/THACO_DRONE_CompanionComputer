#!/bin/bash
set -e

# QGC log aggregation
. /tmp/drone_log.sh 2>/dev/null || true

# Keep ArUco as an explicit legacy ROS2 mode.
export VISION_CONFIG="${VISION_CONFIG:-/app/config/drone.yaml}"
MODE=$(python3 -c 'import os,yaml; print(yaml.safe_load(open(os.environ["VISION_CONFIG"])).get("mode", "aruco"))')
case "$MODE" in
    yolo) drone_info "drone-vision" "Starting YOLO vision mode"; exec python3 /app/scripts/yolo_streamer.py ;;
    aruco) drone_error "drone-vision" "ArUco mode not in this image"; echo "ArUco legacy mode is not packaged in drone-vision:latest; use the dedicated legacy image." >&2; exit 2 ;;
    *) drone_error "drone-vision" "Unsupported mode: ${MODE}"; echo "Unsupported vision mode: $MODE" >&2; exit 1 ;;
esac

# Source ROS2 Jazzy
if [ -f "/opt/ros/jazzy/setup.bash" ]; then
    source /opt/ros/jazzy/setup.bash
fi

echo "=================================================="
echo "  🧠 DRONE VISION & AI PROCESSOR "
echo "  ROS2 Distribution: ${ROS_DISTRO:-jazzy}"
echo "=================================================="

# Lắng nghe topic camera từ container drone-camera (/camera/color/image_raw)
echo "[vision] Khởi động Node xử lý ảnh (ArUco / Landing / Object Detection)..."
exec python3 /app/scripts/vision_processor.py
