#!/bin/bash
# Shared entrypoint for every edge-* image: source ROS 2 + the merged install space, then exec the command.
set -e

[ -f /opt/ros/jazzy/setup.bash ] && . /opt/ros/jazzy/setup.bash
[ -f /opt/cc/setup.bash ] && . /opt/cc/setup.bash

exec "$@"
