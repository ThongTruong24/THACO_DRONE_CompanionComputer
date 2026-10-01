#!/bin/bash
set -e

# QGC log aggregation
. /tmp/drone_log.sh 2>/dev/null || true

echo "=========================================================="
echo "🚁 DRONE CAMERA RTSP STREAMER (C++ SOLID NATIVE ENGINE)"
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

# 2. Khởi động C++ Native Camera Streamer (Zero-Copy & Anti-Overflow)
echo "▶ [2/2] Khởi động C++ Native Camera Streamer..."
while true; do
    echo "   [EXEC] /usr/local/bin/drone_camera_streamer /app/camera.yaml"
    drone_info "drone-camera-rtsp" "Camera streamer starting"
    /usr/local/bin/drone_camera_streamer /app/camera.yaml || {
        echo "⚠️ Streamer thoát với mã lỗi $?, tự động khởi động lại sau 2 giây..."
        sleep 2
    }
done
