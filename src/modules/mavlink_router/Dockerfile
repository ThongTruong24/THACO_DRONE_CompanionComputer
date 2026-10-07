# Stage 1: Builder - mavlink-routerd (upstream) + mavlink-router-controller (ROS 2 node). Build context is the repo root.
FROM edge-ros-builder AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    git pkg-config libc6-dev \
    meson ninja-build python3-pip systemd-dev libsystemd-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Build mavlink-routerd (branch kept at v3 as before; override with --build-arg to pin a tag/commit)
ARG MAVLINK_ROUTER_REF=v3
RUN git clone --depth=1 --branch ${MAVLINK_ROUTER_REF} https://github.com/mavlink-router/mavlink-router.git /build/mavlink-router
WORKDIR /build/mavlink-router
RUN git submodule update --init --recursive
RUN meson setup build --buildtype=release -Dcpp_std=c++17 \
    && ninja -C build \
    && ninja -C build install

# Build the controller node via colcon into the merged install space
WORKDIR /ros2_ws
COPY msg /ros2_ws/msg
COPY src/lib /ros2_ws/src/lib
COPY src/modules/mavlink_router /ros2_ws/src/modules/mavlink_router

RUN . /opt/ros/jazzy/setup.sh && \
    colcon build --merge-install --install-base /opt/cc \
        --base-paths msg src/lib src/modules/mavlink_router \
        --packages-up-to mavlink_router \
        --cmake-args -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF

# Stage 2: Runtime
FROM edge-ros-base AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libc6 iproute2 net-tools iputils-ping \
    tcpdump bash libsystemd0 procps \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/bin/mavlink-routerd /usr/bin/mavlink-routerd
COPY --from=builder /opt/cc /opt/cc

RUN chmod +x /usr/bin/mavlink-routerd \
    && mkdir -p /etc/mavlink-router /var/log/mavlink-router /run/drone

HEALTHCHECK --interval=15s --timeout=5s --start-period=10s --retries=3 \
    CMD bash -c 'pid=$(pgrep -o mavlink-routerd) && [ -n "$pid" ] && ! grep -q "State:.*Z" /proc/$pid/status' || exit 1

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["mavlink-router-controller"]
