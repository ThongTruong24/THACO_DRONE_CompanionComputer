# syntax=docker/dockerfile:1.4
# Base Dockerfile for all Companion Computer edge-* containers (spec 2026-10-04 §5.3, §5.4)

FROM ros:jazzy-ros-core@sha256:8bbec5839a5258d43c8ff8d44c79c87a5c5a929ad419b22d0fa6cd5f196c7818 AS base

ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
    ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST \
    ROS_DOMAIN_ID=0 \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-jazzy-rmw-cyclonedds-cpp \
    && rm -rf /var/lib/apt/lists/*

# DDS stays on loopback (unicast discovery); shared entrypoint sources ROS 2 + /opt/cc for every edge-* image.
COPY platforms/linux/docker/cyclonedds.xml /etc/cc/cyclonedds.xml
COPY platforms/linux/docker/ros_entrypoint.sh /ros_entrypoint.sh
RUN chmod +x /ros_entrypoint.sh
ENV CYCLONEDDS_URI=file:///etc/cc/cyclonedds.xml

FROM base AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    python3-colcon-common-extensions \
    ros-jazzy-ament-cmake \
    ros-jazzy-ament-cmake-gtest \
    ros-jazzy-rosidl-default-generators \
    ros-jazzy-rosidl-default-runtime \
    ros-jazzy-sensor-msgs \
    ros-jazzy-std-srvs \
    && rm -rf /var/lib/apt/lists/*
