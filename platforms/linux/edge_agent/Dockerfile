# ── Stage 1: Builder ──────────────────────────────────────────────────────────
FROM edge-ros-builder AS builder

WORKDIR /ros2_ws
COPY cmake /ros2_ws/cmake
COPY msg /ros2_ws/msg
COPY src/lib /ros2_ws/src/lib
COPY src/modules/config_manager /ros2_ws/src/modules/config_manager
COPY src/modules/mavlink /ros2_ws/src/modules/mavlink
COPY platforms/linux/edge_agent /ros2_ws/platforms/linux/edge_agent

RUN . /opt/ros/jazzy/setup.sh && \
    colcon build --merge-install --install-base /opt/cc \
        --base-paths msg src/lib src/modules/config_manager src/modules/mavlink platforms/linux/edge_agent \
        --packages-up-to edge_agent \
        --cmake-args -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF

# ── Stage 2: Runtime ──────────────────────────────────────────────────────────
FROM edge-ros-base AS runtime

COPY --from=builder /opt/cc /opt/cc

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["edge_agent"]
