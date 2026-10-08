#!/bin/bash
# Runs inside edge-camera (via /ros_entrypoint.sh, which already sourced ROS 2 and /opt/cc).
# "$@" is forwarded to the node, e.g. --ros-args --params-file /app/config/camera_streamer.yaml
set -e

echo "==========================================================" 
echo "🚁 DRONE CAMERA RTSP STREAMER (C++ ROS 2 NODE)"
echo "=========================================================="

# 1. Khởi động MediaMTX RTSP Server
echo "▶ [1/2] Khởi động MediaMTX RTSP Server..."
/usr/local/bin/mediamtx /app/mediamtx.yml &
MEDIAMTX_PID=$!

# Đợi cổng RTMP 1935 và RTSP 8554 thực sự sẵn sàng
echo "   Chờ MediaMTX port 1935/8554 sẵn sàng..."
for i in $(seq 1 30); do
    if ss -tln 2>/dev/null | grep -q ":1935"; then
        echo "   ✓ MediaMTX RTMP:1935 đã sẵn sàng!"
        break
    fi
    sleep 0.2
done

if ! kill -0 ${MEDIAMTX_PID} 2>/dev/null; then
    echo "❌ Lỗi: MediaMTX không thể khởi động! Kiểm tra mediamtx.yml"
    exit 1
fi

# 2. Xác định cấu hình Camera Hardware Model
if [ -z "$CAMERA_MODEL" ] && [ -f /app/config/camera_streamer.yaml ]; then
    CAMERA_MODEL=$(grep -E '^[[:space:]]*camera_model:' /app/config/camera_streamer.yaml | awk '{print $2}' | tr -d '"\r\n')
fi
CAMERA_MODEL="${CAMERA_MODEL:-realsense_d435i}"

CAMERA_PARAM_ARGS=()
if [ -f "/app/config/cameras/${CAMERA_MODEL}.yaml" ]; then
    echo "   ✓ Nạp hồ sơ camera: /app/config/cameras/${CAMERA_MODEL}.yaml"
    CAMERA_PARAM_ARGS=("--ros-args" "--params-file" "/app/config/cameras/${CAMERA_MODEL}.yaml" "--")
else
    echo "   ℹ Không tìm thấy /app/config/cameras/${CAMERA_MODEL}.yaml, dùng tham số mặc định của node"
fi

# 3. Khởi động node camera_streamer; thoát thì dựng lại (MediaMTX giữ nguyên)
echo "▶ [2/2] Khởi động camera_streamer_node..."
while true; do
    echo "   [EXEC] camera_streamer_node ${CAMERA_PARAM_ARGS[@]} $*"
    camera_streamer_node "${CAMERA_PARAM_ARGS[@]}" "$@" || {
        echo "⚠️ Streamer thoát với mã lỗi $?, tự động khởi động lại sau 2 giây..."
        sleep 2
    }
done
