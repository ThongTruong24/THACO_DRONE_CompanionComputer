#!/usr/bin/env bash
# ==============================================================================
# Script Cross-Compile C++ Camera Streamer sang ARM64 (Raspberry Pi 5) trên WSL2
# ==============================================================================

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DRONE_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

echo "======================================================================"
echo " 🛠️  Cross-Compiling C++ Native Binaries sang ARM64 (aarch64) trên WSL2"
echo "======================================================================"

# Kiểm tra docker và buildx
if ! command -v docker &>/dev/null; then
    echo "[LỖI] Không tìm thấy Docker trên hệ thống."
    exit 1
fi

# Tự động khắc phục lỗi stale bind mount của Docker Desktop trên WSL2
if ! docker buildx inspect drone-builder &>/dev/null; then
    echo ">>> Khởi tạo builder Buildx ARM64 mới (drone-builder)..."
    docker buildx create --name drone-builder --use --driver docker-container
elif ! docker buildx inspect drone-builder --bootstrap &>/dev/null; then
    echo ">>> Phát hiện stale container builder sau khi WSL/Docker restart. Đang tái tạo drone-builder..."
    docker buildx rm -f drone-builder 2>/dev/null || true
    docker buildx create --name drone-builder --use --driver docker-container
fi
docker buildx use drone-builder

mkdir -p "${DRONE_ROOT}/src_native/bin"

echo ">>> Bắt đầu biên dịch chéo bằng Docker Buildx (ARM64 Cortex-A76)..."
docker buildx build \
    --platform linux/arm64 \
    --target export \
    --output type=local,dest="${DRONE_ROOT}/src_native/bin/" \
    -f "${DRONE_ROOT}/src/camera-stream-controller/Dockerfile.cross" \
    "${DRONE_ROOT}/src/camera-stream-controller"

chmod +x "${DRONE_ROOT}/src_native/bin/drone_camera_streamer" 2>/dev/null || true

echo ""
echo "======================================================================"
echo " [THÀNH CÔNG] Đã biên dịch xong binary ARM64:"
file "${DRONE_ROOT}/src_native/bin/drone_camera_streamer" 2>/dev/null || ls -l "${DRONE_ROOT}/src_native/bin/drone_camera_streamer"
echo "======================================================================"
