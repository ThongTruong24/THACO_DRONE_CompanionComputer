# ── Stage 1: Builder ──────────────────────────────────────────────────────────
FROM edge-ros-builder AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Build tools + RealSense dev + GStreamer dev
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl ca-certificates tar pkg-config \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    ros-jazzy-librealsense2 \
    && rm -rf /var/lib/apt/lists/*

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

# Build camera_streamer via colcon into the merged install space (context = repo root)
WORKDIR /ros2_ws
COPY msg /ros2_ws/msg
COPY src/lib /ros2_ws/src/lib
COPY src/drivers/camera /ros2_ws/src/drivers/camera
COPY src/modules/camera_streamer /ros2_ws/src/modules/camera_streamer

RUN . /opt/ros/jazzy/setup.sh && \
    colcon build --merge-install --install-base /opt/cc \
        --base-paths msg src/lib src/modules/camera_streamer \
        --packages-up-to camera_streamer \
        --cmake-args -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF

# ── Stage 2: Runtime ──────────────────────────────────────────────────────────
FROM edge-ros-base AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-jazzy-sensor-msgs \
    ros-jazzy-librealsense2 \
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

COPY --from=builder /usr/local/bin/mediamtx /usr/local/bin/mediamtx
COPY --from=builder /opt/cc /opt/cc

WORKDIR /app
COPY src/modules/camera_streamer/mediamtx.yml /app/mediamtx.yml
COPY src/modules/camera_streamer/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh /usr/local/bin/mediamtx

# Node parameters come from `--params-file` (compose mounts config/camera_streamer.yaml).
ENTRYPOINT ["/ros_entrypoint.sh", "/app/entrypoint.sh"]
