# syntax=docker/dockerfile:1
# This dependency image supplies Torch, Ultralytics, OpenCV and GStreamer.
# It is consumed at build time; Pi pulls only the final drone-vision-new image.
ARG VISION_BASE_IMAGE=ghcr.io/thongtruong24/drone-vision@sha256:9e4f40d9c8da4afee7e3d4e711d04eae11dbfb6940d8e93ca94f05a84007d2d6
FROM --platform=$BUILDPLATFORM ubuntu:24.04 AS native-build
ARG TARGETARCH
RUN apt-get update && apt-get install -y --no-install-recommends cmake make g++-aarch64-linux-gnu ca-certificates \
    && rm -rf /var/lib/apt/lists/*
COPY src/cc-agent/mavlink/ /build/mavlink/
COPY src/cc-agent/include/nlohmann/ /build/json/nlohmann/
COPY src/drone-vision-new/native/ /build/native/
RUN test "${TARGETARCH}" = arm64 \
    && cmake -S /build/native -B /build/out -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
    -DTHACO_INCLUDE_DIR=/build/mavlink -DJSON_INCLUDE_DIR=/build/json \
    && cmake --build /build/out -j2

FROM ${VISION_BASE_IMAGE} AS runtime
ARG SOURCE_REVISION=unknown
LABEL org.opencontainers.image.title="drone-vision-new" \
      org.opencontainers.image.revision="${SOURCE_REVISION}"
WORKDIR /app
# Remove inherited application files. Runtime application ownership is entirely this image.
RUN rm -rf /app/scripts /app/config /app/models /app/entrypoint.sh
COPY --from=native-build /build/out/mavlink-bridge /usr/local/bin/mavlink-bridge
COPY --from=native-build /build/mavlink/ /usr/local/include/mavlink/
COPY src/drone-vision-new/scripts/ /app/scripts/
COPY src/drone-vision-new/config/ /app/config/
COPY src/drone-vision-new/entrypoint.sh /app/entrypoint.sh
COPY src/drone-vision/yolo26n.pt /app/models/yolo26n.pt
COPY src/drone-vision-new/tests/test_*.py /app/tests/
ENV PYTHONUNBUFFERED=1 PYTHONDONTWRITEBYTECODE=1 VISION_CONFIG=/app/config/drone.yaml
RUN chmod +x /app/entrypoint.sh /usr/local/bin/mavlink-bridge \
    && python3 /app/scripts/main.py --check
ENTRYPOINT ["/app/entrypoint.sh"]
