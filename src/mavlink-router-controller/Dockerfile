# Stage 1: Builder - Build MAVLink Router & Supervisor Controller
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    git cmake build-essential pkg-config libc6-dev \
    meson ninja-build python3-pip systemd-dev libsystemd-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Build mavlink-routerd
RUN git clone --depth=1 https://github.com/mavlink-router/mavlink-router.git /build/mavlink-router
WORKDIR /build/mavlink-router
RUN git submodule update --init --recursive
RUN meson setup build --buildtype=release \
    && ninja -C build \
    && ninja -C build install

# Build mavlink-router-controller (C++20 Supervisor)
WORKDIR /build/controller
COPY . .
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && \
    make install

# Stage 2: Runtime
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libc6 iproute2 net-tools iputils-ping \
    tcpdump bash libsystemd0 procps \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/bin/mavlink-routerd /usr/bin/mavlink-routerd
COPY --from=builder /usr/local/bin/mavlink-router-controller /usr/local/bin/mavlink-router-controller

RUN chmod +x /usr/bin/mavlink-routerd /usr/local/bin/mavlink-router-controller \
    && mkdir -p /etc/mavlink-router /var/log/mavlink-router /run/drone

HEALTHCHECK --interval=15s --timeout=5s --start-period=10s --retries=3 \
    CMD pgrep mavlink-routerd > /dev/null || exit 1

ENTRYPOINT ["/usr/local/bin/mavlink-router-controller"]
CMD ["/run/drone/router.sock", "/run/drone/hw_manager.sock"]
