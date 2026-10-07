# ── Stage 1: Builder ──────────────────────────────────────────────────────────
FROM edge-ros-builder AS builder

WORKDIR /ros2_ws
COPY msg /ros2_ws/msg
COPY src/modules/networking /ros2_ws/src/modules/networking

RUN . /opt/ros/jazzy/setup.sh && \
    colcon build --merge-install --install-base /opt/cc \
        --base-paths msg src/modules/networking \
        --packages-up-to networking \
        --cmake-args -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF

# ── Stage 2: Runtime ──────────────────────────────────────────────────────────
FROM edge-ros-base AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-jazzy-sensor-msgs ros-jazzy-std-srvs \
    hostapd \
    dnsmasq \
    rfkill \
    iproute2 \
    iptables \
    iw \
    wireless-tools \
    wpasupplicant \
    avahi-daemon \
    avahi-utils \
    psmisc \
    bash \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /opt/cc /opt/cc

WORKDIR /app
COPY src/modules/networking/dnsmasq.conf /app/dnsmasq.conf
COPY src/modules/networking/50-drone.yaml /etc/netplan/50-drone.yaml
COPY src/modules/networking/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh

ENTRYPOINT ["/ros_entrypoint.sh", "/app/entrypoint.sh"]
