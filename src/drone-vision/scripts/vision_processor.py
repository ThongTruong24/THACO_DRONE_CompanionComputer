#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
vision_processor.py - Node xử lý ảnh mẫu (ArUco Marker Landing Target)
- Tương thích cả OpenCV 4.6 và 4.7+
- Lắng nghe /camera/camera/color/image_raw hoặc /camera/color/image_raw
- Tìm kiếm ArUco Marker dùng cho hạ cánh chính xác (Precision Landing)
- Gửi tọa độ lệch góc qua cổng MAVLink nội bộ 127.0.0.1:14551
"""

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import cv2
import time
import os

try:
    from pymavlink import mavutil
    MAVLINK_AVAILABLE = True
except ImportError:
    MAVLINK_AVAILABLE = False


class VisionProcessor(Node):
    def __init__(self):
        super().__init__('drone_vision_processor')
        self.bridge = CvBridge()
        self.get_logger().info("Khởi động Vision Processor (ArUco Landing Hook)")

        # Kết nối MAVLink tới cổng nội bộ của mavlink-router
        self.mav_conn = None
        if MAVLINK_AVAILABLE:
            try:
                self.mav_conn = mavutil.mavlink_connection('udpout:127.0.0.1:14551')
                self.get_logger().info("Đã kết nối MAVLink tới udpout:127.0.0.1:14551")
            except Exception as e:
                self.get_logger().warn(f"Chưa kết nối được MAVLink: {e}")

        # Khởi tạo ArUco detector (tương thích cả 4.6 và 4.7+)
        try:
            self.aruco_dict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
            self.aruco_params = cv2.aruco.DetectorParameters()
            self.detector = cv2.aruco.ArucoDetector(self.aruco_dict, self.aruco_params)
            self.legacy = False
        except AttributeError:
            self.aruco_dict = cv2.aruco.Dictionary_get(cv2.aruco.DICT_4X4_50)
            self.aruco_params = cv2.aruco.DetectorParameters_create()
            self.detector = None
            self.legacy = True

        self.sub1 = self.create_subscription(
            Image,
            '/camera/camera/color/image_raw',
            self.image_callback,
            10
        )
        self.sub2 = self.create_subscription(
            Image,
            '/camera/color/image_raw',
            self.image_callback,
            10
        )

    def image_callback(self, msg):
        try:
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
            gray = cv2.cvtColor(cv_image, cv2.COLOR_BGR2GRAY)

            if self.legacy:
                corners, ids, _ = cv2.aruco.detectMarkers(
                    gray, self.aruco_dict, parameters=self.aruco_params
                )
            else:
                corners, ids, _ = self.detector.detectMarkers(gray)

            if ids is not None:
                for i in range(len(ids)):
                    marker_id = ids[i][0]
                    c = corners[i][0]
                    center_x = int(c[:, 0].mean())
                    center_y = int(c[:, 1].mean())

                    h, w = gray.shape
                    dx = center_x - (w / 2)
                    dy = center_y - (h / 2)

                    self.get_logger().info(
                        f"[ArUco #{marker_id}] Phát hiện tại ({center_x}, {center_y}) | Lệch: dx={dx:.1f}, dy={dy:.1f}"
                    )

                    if self.mav_conn:
                        angle_x = (dx / w) * 1.22
                        angle_y = (dy / h) * 0.75
                        self.mav_conn.mav.landing_target_send(
                            int(time.time() * 1e6),
                            marker_id,
                            0,
                            angle_x,
                            angle_y,
                            1.0,
                            0.0, 0.0
                        )
        except Exception:
            pass


def main(args=None):
    rclpy.init(args=args)
    node = VisionProcessor()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
