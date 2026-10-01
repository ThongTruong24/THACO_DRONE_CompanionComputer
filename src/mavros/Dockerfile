FROM ros:jazzy-ros-base

ENV DEBIAN_FRONTEND=noninteractive \
    ROS_DOMAIN_ID=0 \
    RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
      ros-jazzy-mavros \
      ros-jazzy-mavros-extras \
      ros-jazzy-rmw-cyclonedds-cpp \
      libyaml-cpp-dev \
      nlohmann-json3-dev \
      ros-jazzy-ament-cmake-gtest \
      geographiclib-tools procps \
    && /opt/ros/jazzy/lib/mavros/install_geographiclib_datasets.sh \
    && rm -rf /var/lib/apt/lists/*

# Install THACO AgriDrone MAVLink C++ headers
COPY mavlink/agridrone-thaco-mavlink/include/mavlink/v2.0/thaco_common /opt/ros/jazzy/include/mavlink/v2.0/thaco_common

# Build and install plugin_agridrone
COPY ros2ws/cc_interfaces /ws/src/cc_interfaces
COPY ros2ws/cc_telemetry /ws/src/cc_telemetry
COPY mavlink/plugin_agridrone /ws/src/plugin_agridrone
RUN . /opt/ros/jazzy/setup.sh \
    && cd /ws \
    && colcon build --merge-install --install-base /opt/cc_ws --packages-up-to cc_telemetry plugin_agridrone \
    && . /opt/cc_ws/setup.sh && colcon test --merge-install --install-base /opt/cc_ws --packages-select cc_telemetry --return-code-on-test-failure \
    && colcon test-result --verbose \
    && mkdir -p /opt/cc_ws/lib/cc_telemetry \
    && (test -f /opt/cc_ws/lib/cc_telemetry_node && ln -sf /opt/cc_ws/lib/cc_telemetry_node /opt/cc_ws/lib/cc_telemetry/cc_telemetry_node || true) \
    && rm -rf /ws
COPY ros2ws/cc_telemetry/config/cc_telemetry.yaml /etc/cc-telemetry/cc_telemetry.yaml

COPY entrypoint.sh /entrypoint.sh
COPY telemetry_demo.py /usr/local/bin/mavros-telemetry-demo
RUN chmod 0755 /entrypoint.sh /usr/local/bin/mavros-telemetry-demo

HEALTHCHECK --interval=20s --timeout=5s --start-period=25s --retries=3 \
    CMD pgrep -f 'mavros_node' >/dev/null && pgrep -f 'cc_telemetry_node' >/dev/null || exit 1

ENTRYPOINT ["/entrypoint.sh"]
