# ── Stage 1: Builder ──────────────────────────────────────────────────────────
# Build C++ native streamer với đầy đủ dev headers.
# Stage này KHÔNG được giữ lại trong runtime image.
FROM ros:jazzy-ros-base AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Build tools + RealSense dev + GStreamer dev
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
    cmake build-essential pkg-config curl ca-certificates tar \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    libyaml-cpp-dev \
    ros-jazzy-librealsense2 \
    && rm -rf /var/lib/apt/lists/*

ENV PKG_CONFIG_PATH=/opt/ros/jazzy/lib/aarch64-linux-gnu/pkgconfig
ENV CMAKE_PREFIX_PATH=/opt/ros/jazzy

# Tải MediaMTX binary
ARG MEDIAMTX_VERSION=v1.9.3
ARG TARGETARCH
RUN case "${TARGETARCH}" in \
        "arm64") MTX_ARCH="linux_arm64v8" ;; \
        "amd64") MTX_ARCH="linux_amd64" ;; \
        "arm/v7") MTX_ARCH="linux_armv7" ;; \
        *) MTX_ARCH="linux_arm64v8" ;; \
    esac && \
    curl -fsSL -o /tmp/mediamtx.tar.gz \
        "https://github.com/bluenviron/mediamtx/releases/download/${MEDIAMTX_VERSION}/mediamtx_${MEDIAMTX_VERSION}_${MTX_ARCH}.tar.gz" && \
    tar -xzf /tmp/mediamtx.tar.gz -C /usr/local/bin mediamtx && \
    chmod +x /usr/local/bin/mediamtx && \
    rm -f /tmp/mediamtx.tar.gz

# Biên dịch C++ binary
WORKDIR /app
COPY include/ /app/include/
COPY src/     /app/src/
COPY CMakeLists.txt /app/CMakeLists.txt
RUN mkdir -p /app/build && cd /app/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc --ignore=2 2>/dev/null || echo 2) && \
    make install && \
    rm -rf /app/build

# ── Stage 2: Runtime ──────────────────────────────────────────────────────────
# Chỉ giữ binary đã build và thư viện runtime .so tối thiểu.
# Base: ubuntu:24.04 (không phải ros:jazzy) để giảm ~1 GB.
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

# Runtime libs: GStreamer plugins and yaml-cpp. librealsense is copied from the ROS builder,
# because Ubuntu Noble ARM64 repositories do not provide a standalone librealsense2 package.
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y --no-install-recommends \
    libyaml-cpp0.8 \
    libgstreamer1.0-0 \
    libgstreamer-plugins-base1.0-0 \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-rtsp \
    libssl3 \
    iproute2 \
    ca-certificates \
    bash \
    && rm -rf /var/lib/apt/lists/*

# Copy binaries and the ROS-packaged RealSense runtime from the builder.
COPY --from=builder /usr/local/bin/mediamtx              /usr/local/bin/mediamtx
COPY --from=builder /usr/local/bin/drone_camera_streamer /usr/local/bin/drone_camera_streamer
COPY --from=builder /opt/ros/jazzy/lib/aarch64-linux-gnu/librealsense2.so* /usr/local/lib/
RUN ldconfig

WORKDIR /app
COPY scripts/     /app/scripts/
COPY camera.yaml  /app/camera.yaml
COPY mediamtx.yml /app/mediamtx.yml
COPY entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh /usr/local/bin/mediamtx /usr/local/bin/drone_camera_streamer

ENTRYPOINT ["/app/entrypoint.sh"]
