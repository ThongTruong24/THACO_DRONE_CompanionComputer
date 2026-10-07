#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"

# Test 1: extracts camera_model correctly from camera_streamer.yaml
STREAMER_YAML="$REPO_ROOT/src/modules/camera_streamer/config/camera_streamer.yaml"
MODEL=$(grep -E '^[[:space:]]*camera_model:' "$STREAMER_YAML" | awk '{print $2}' | tr -d '"\r\n')
if [ "$MODEL" != "realsense_d435i" ]; then
    echo "FAIL: Expected realsense_d435i from streamer config, got '$MODEL'"
    exit 1
fi

# Test 2: verifies corresponding driver config exists
CAMERA_CONFIG="$REPO_ROOT/src/drivers/camera/config/${MODEL}.yaml"
if [ ! -f "$CAMERA_CONFIG" ]; then
    echo "FAIL: Camera config file does not exist: $CAMERA_CONFIG"
    exit 1
fi

# Test 3: extracts camera_model correctly from vision.yaml
VISION_YAML="$REPO_ROOT/src/modules/vision/config/vision.yaml"
VISION_MODEL=$(grep -E '^[[:space:]]*camera_model:' "$VISION_YAML" | awk '{print $2}' | tr -d '"\r\n')
if [ "$VISION_MODEL" != "realsense_d435i" ]; then
    echo "FAIL: Expected realsense_d435i from vision config, got '$VISION_MODEL'"
    exit 1
fi

echo "Entrypoint composition test PASSED"
