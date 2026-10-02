#!/usr/bin/env bash
# Test-only wrapper: use packaged ARM64 helper with host's isolated router fixture.
set -euo pipefail
exec docker run --rm -i --platform linux/arm64 --network host \
  --entrypoint /usr/local/bin/mavlink-bridge \
  "${VISION_CAMERA_TEST_IMAGE:-ghcr.io/thongtruong24/drone-vision-new:camera-alias-local-test}" "$@"
