# ── Stage 1: Python inference dependencies ────────────────────────────────
FROM edge-ros-base AS deps

ENV DEBIAN_FRONTEND=noninteractive
ENV YOLO_AUTOINSTALL=false

RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-venv python3-pip python3-yaml python3-opencv ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv --system-site-packages /opt/vision && \
    /opt/vision/bin/pip install --no-cache-dir 'numpy<2' torch==2.6.0 torchvision==0.21.0 && \
    /opt/vision/bin/pip install --no-cache-dir --no-deps ultralytics==8.4.14

# ── Stage 2: ROS package build ────────────────────────────────────────────
FROM edge-ros-builder AS builder

WORKDIR /ros2_ws
COPY msg /ros2_ws/msg
COPY src/lib/frame_pool /ros2_ws/src/lib/frame_pool
COPY src/lib/hrt /ros2_ws/src/lib/hrt
COPY src/modules/vision_v1 /ros2_ws/src/modules/vision_v1

RUN . /opt/ros/jazzy/setup.sh && \
    colcon build --merge-install --install-base /opt/cc \
        --base-paths msg src/lib/frame_pool src/modules/vision_v1 \
        --packages-up-to vision_v1 \
        --cmake-args -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF && \
    sed -i '1s|^#!.*python.*$|#!/opt/vision/bin/python3|' /opt/cc/lib/vision_v1/*

# ── Stage 3: Runtime ──────────────────────────────────────────────────────
FROM deps AS runtime

ENV PATH=/opt/vision/bin:${PATH}
ENV YOLO_CONFIG_DIR=/tmp/ultralytics

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-jazzy-sensor-msgs ros-jazzy-std-srvs \
    python3-gi gir1.2-gstreamer-1.0 \
    gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav \
    libssl3 bash \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /opt/cc /opt/cc
COPY src/modules/vision_v1/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh

WORKDIR /app/models
ENTRYPOINT ["/ros_entrypoint.sh", "/app/entrypoint.sh"]
