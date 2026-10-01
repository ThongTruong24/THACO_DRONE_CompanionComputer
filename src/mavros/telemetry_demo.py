#!/usr/bin/env python3
"""Read-only MAVROS telemetry observer for the PX4 companion container."""

from __future__ import annotations

import math
from dataclasses import dataclass
from typing import Optional

import rclpy
from geometry_msgs.msg import PoseStamped
from mavros_msgs.msg import State
from rclpy.node import Node
from sensor_msgs.msg import BatteryState, Imu, NavSatFix



@dataclass
class TelemetrySnapshot:
    state: Optional[State] = None
    imu: Optional[Imu] = None
    gps: Optional[NavSatFix] = None
    battery: Optional[BatteryState] = None
    local_pose: Optional[PoseStamped] = None


def format_number(value: float, precision: int = 2) -> str:
    """Return a telemetry number or N/A for unavailable/non-finite data."""
    return f"{value:.{precision}f}" if math.isfinite(value) else "N/A"


class TelemetryDemo(Node):
    """Subscribe to MAVROS only; this node never publishes to the FC."""

    def __init__(self) -> None:
        super().__init__("mavros_telemetry_demo")
        self.snapshot = TelemetrySnapshot()
        self.last_state_signature: Optional[tuple[bool, bool, str, int]] = None

        self.create_subscription(State, "/drone_mavros/state", self.on_state, 10)
        self.create_subscription(Imu, "/drone_mavros/imu/data", self.on_imu, 10)
        self.create_subscription(NavSatFix, "/drone_mavros/global_position/global", self.on_gps, 10)
        self.create_subscription(BatteryState, "/drone_mavros/battery", self.on_battery, 10)
        self.create_subscription(PoseStamped, "/drone_mavros/local_position/pose", self.on_local_pose, 10)

        self.create_timer(1.0, self.log_summary)
        self.get_logger().info(
            "Telemetry observer started (read-only): state, IMU, GPS, battery, local pose."
        )

    def on_state(self, message: State) -> None:
        self.snapshot.state = message
        signature = (message.connected, message.armed, message.mode, message.system_status)
        if signature != self.last_state_signature:
            self.last_state_signature = signature
            self.get_logger().info(
                "FC state changed: connected=%s armed=%s mode=%s system_status=%d" % signature
            )

    def on_imu(self, message: Imu) -> None:
        self.snapshot.imu = message

    def on_gps(self, message: NavSatFix) -> None:
        self.snapshot.gps = message

    def on_battery(self, message: BatteryState) -> None:
        self.snapshot.battery = message

    def on_local_pose(self, message: PoseStamped) -> None:
        self.snapshot.local_pose = message

    def log_summary(self) -> None:
        state, gps, battery, imu, local_pose = (
            self.snapshot.state,
            self.snapshot.gps,
            self.snapshot.battery,
            self.snapshot.imu,
            self.snapshot.local_pose,
        )
        state_text = "N/A" if state is None else (
            f"connected={state.connected} armed={state.armed} mode={state.mode}"
        )
        gps_text = "NO_MESSAGE" if gps is None else (
            "NO_FIX lat=%s lon=%s (not usable)" % (
                format_number(gps.latitude, 7), format_number(gps.longitude, 7)
            ) if gps.status.status < 0 else "FIX lat=%s lon=%s alt=%sm" % (
                format_number(gps.latitude, 7), format_number(gps.longitude, 7),
                format_number(gps.altitude),
            )
        )
        battery_text = "N/A" if battery is None else "voltage=%sV remaining=%s%%" % (
            format_number(battery.voltage), format_number(battery.percentage * 100.0),
        )
        imu_text = "NO_MESSAGE" if imu is None else "gyro=(%s,%s,%s) accel=(%s,%s,%s)m/s2" % (
            format_number(imu.angular_velocity.x), format_number(imu.angular_velocity.y), format_number(imu.angular_velocity.z),
            format_number(imu.linear_acceleration.x), format_number(imu.linear_acceleration.y), format_number(imu.linear_acceleration.z),
        )
        local_text = "NO_MESSAGE" if local_pose is None else "x=%s y=%s z=%sm" % (
            format_number(local_pose.pose.position.x), format_number(local_pose.pose.position.y), format_number(local_pose.pose.position.z),
        )
        self.get_logger().info(
            f"Telemetry: state[{state_text}] local[{local_text}] gps[{gps_text}] "
            f"battery[{battery_text}] imu[{imu_text}]"
        )



def main() -> None:
    rclpy.init()
    node = TelemetryDemo()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()