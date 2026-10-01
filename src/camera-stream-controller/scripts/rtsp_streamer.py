#!/usr/bin/env python3
"""
rtsp_streamer.py - Ultra-Low-Latency H.264 Streamer cho Drone Edge Computing
Tích hợp MediaMTX RTSP Server + RealSense D435i ROS2 Driver (All-in-One Container)

Cải tiến độ ổn định (Zero-Loss / Anti-Overflow / Auto-Recovery):
1. Chống tràn bộ đệm (leaky=downstream): Drop frame cũ nhất nếu encoder bận, không bao giờ tích lũy buffer.
2. Tự động phục hồi (Auto-Recovery Watchdog): Tự động phát hiện lỗi pipeline và re-init trong 1 giây.
3. Tối ưu hóa CPU: Không gọi cv2.resize nếu kích thước khung hình đã chuẩn 640x480.
4. Standby Mode mượt mà: Phát test pattern khi camera chưa bật, tự động nhường chỗ khi camera online.
"""

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import cv2
import numpy as np
import os
import time
import yaml

DEFAULT_WIDTH = 640
DEFAULT_HEIGHT = 480
DEFAULT_FPS = 30
DEFAULT_BITRATE = 1500
RTMP_URL = os.environ.get("RTMP_URL", "rtmp://127.0.0.1:1935/camera")

CONFIG_PATHS = [
    "/app/camera.yaml",
    "/app/config/camera.yaml",
    os.path.join(os.path.dirname(__file__), "../camera.yaml")
]


class RTSPStreamer(Node):
    def __init__(self):
        super().__init__('drone_rtsp_streamer')
        self.bridge = CvBridge()
        self.writer = None
        self.frame_count = 0
        self.last_frame_time = 0
        self.last_config_check = time.time()
        self.last_reinit_attempt = 0
        self.write_fail_count = 0
        self.is_recovering = False

        self.load_config()

        self.get_logger().info(
            f"Khởi động Ultra-Low-Latency RTSP Streamer: {RTMP_URL} "
            f"({self.width}x{self.height}@{self.fps}fps, {self.bitrate}kbps) | "
            f"Rotation: {self.rotation} | Intra-Refresh: {self.intra_refresh}"
        )

        self.init_writer()

        # Lắng nghe topic từ realsense2_camera
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

        # Timer dự phòng khi camera chưa sẵn sàng (test pattern) & Watchdog kiểm tra sức khỏe
        self.timer = self.create_timer(1.0 / self.fps, self.standby_timer)

    def load_config(self):
        """Đọc tham số cấu hình từ file YAML hoặc biến môi trường"""
        self.rotation = os.environ.get("CAMERA_ROTATION", "180")
        self.width = DEFAULT_WIDTH
        self.height = DEFAULT_HEIGHT
        self.fps = DEFAULT_FPS
        self.bitrate = DEFAULT_BITRATE
        self.key_int_max = 30
        self.intra_refresh = True

        for path in CONFIG_PATHS:
            if os.path.exists(path):
                try:
                    with open(path, 'r', encoding='utf-8') as f:
                        cfg = yaml.safe_load(f)
                    if isinstance(cfg, dict):
                        cam_cfg = cfg.get('camera', cfg)
                        if 'rotation' in cam_cfg:
                            self.rotation = str(cam_cfg['rotation'])
                        if 'width' in cam_cfg:
                            self.width = int(cam_cfg['width'])
                        if 'height' in cam_cfg:
                            self.height = int(cam_cfg['height'])
                        if 'fps' in cam_cfg:
                            self.fps = int(cam_cfg['fps'])
                        if 'bitrate' in cam_cfg:
                            self.bitrate = int(cam_cfg['bitrate'])
                        if 'key_int_max' in cam_cfg:
                            self.key_int_max = int(cam_cfg['key_int_max'])
                        if 'intra_refresh' in cam_cfg:
                            self.intra_refresh = bool(cam_cfg['intra_refresh'])
                        break
                except Exception as e:
                    self.get_logger().warn(f"Lỗi đọc {path}: {e}")

    def apply_rotation(self, img):
        """Xoay hoặc lật hình ảnh theo cấu hình"""
        rot = str(self.rotation).strip().lower()
        if rot in ("0", "none", "no"):
            return img
        elif rot in ("180", "180.0"):
            return cv2.rotate(img, cv2.ROTATE_180)
        elif rot in ("90", "90.0", "cw"):
            return cv2.rotate(img, cv2.ROTATE_90_CLOCKWISE)
        elif rot in ("270", "270.0", "ccw"):
            return cv2.rotate(img, cv2.ROTATE_90_COUNTERCLOCKWISE)
        elif rot in ("hflip", "horizontal", "h"):
            return cv2.flip(img, 1)
        elif rot in ("vflip", "vertical", "v"):
            return cv2.flip(img, 0)
        return img

    def init_writer(self):
        """Khởi tạo GStreamer VideoWriter với cơ chế chống tràn bộ đệm"""
        now = time.time()
        if now - self.last_reinit_attempt < 1.0:
            return False
        self.last_reinit_attempt = now

        if self.writer is not None:
            try:
                self.writer.release()
            except Exception:
                pass
            self.writer = None

        intra_flag = "intra-refresh=true" if self.intra_refresh else ""

        # GStreamer pipeline chống tràn buffer (leaky=downstream) và độ trễ siêu thấp
        gst_pipeline = (
            f"appsrc is-live=true do-timestamp=true format=time block=false max-buffers=2 leaky-type=downstream ! "
            f"video/x-raw,format=BGR,width={self.width},height={self.height},framerate={self.fps}/1 ! "
            f"queue max-size-buffers=2 max-size-time=0 max-size-bytes=0 leaky=downstream ! "
            f"videoconvert ! "
            f"video/x-raw,format=I420 ! "
            f"x264enc tune=zerolatency speed-preset=ultrafast bitrate={self.bitrate} "
            f"key-int-max={self.key_int_max} {intra_flag} vbv-buf-capacity=100 byte-stream=true aud=true "
            f"sliced-threads=true threads=4 ! "
            f"video/x-h264,profile=baseline ! "
            f"h264parse config-interval=-1 ! "
            f"flvmux streamable=true ! "
            f"rtmpsink location={RTMP_URL} sync=false"
        )

        try:
            self.writer = cv2.VideoWriter(
                gst_pipeline,
                cv2.CAP_GSTREAMER,
                0,
                float(self.fps),
                (self.width, self.height),
                True
            )
        except Exception as e:
            self.get_logger().error(f"Lỗi khởi tạo VideoWriter: {e}")
            self.writer = None

        if self.writer is None or not self.writer.isOpened():
            self.get_logger().warn(f"Chưa kết nối được RTMP MediaMTX ({RTMP_URL}), sẽ tự động kết nối lại...")
            return False
        else:
            self.get_logger().info(
                f"✓ VideoWriter kết nối thành công: {self.width}x{self.height}@{self.fps}fps | "
                f"Bitrate: {self.bitrate}kbps | Anti-Overflow: Active"
            )
            self.write_fail_count = 0
            return True

    def write_frame_safe(self, frame):
        """Ghi frame an toàn với watchdog phát hiện lỗi pipeline"""
        if self.writer is None or not self.writer.isOpened():
            if not self.init_writer():
                return False

        try:
            self.writer.write(frame)
            self.write_fail_count = 0
            return True
        except Exception as e:
            self.write_fail_count += 1
            if self.write_fail_count >= 15:
                self.get_logger().warn(f"Phát hiện lỗi pipeline liên tiếp ({e}), tiến hành Auto-Recovery...")
                self.init_writer()
            return False

    def image_callback(self, msg):
        try:
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')

            # Áp dụng xoay/lật hình ảnh
            cv_image = self.apply_rotation(cv_image)

            # Resize CHỈ KHI kích thước không khớp (tiết kiệm CPU)
            if cv_image.shape[1] != self.width or cv_image.shape[0] != self.height:
                cv_image = cv2.resize(cv_image, (self.width, self.height))

            if self.write_frame_safe(cv_image):
                self.last_frame_time = time.time()
                self.frame_count += 1
                if self.frame_count % (self.fps * 5) == 0:
                    mbgr = cv_image.mean(axis=(0, 1))
                    self.get_logger().info(
                        f"Stream #{self.frame_count} (Rotation: {self.rotation}): mean BGR=[{mbgr[0]:.1f}, {mbgr[1]:.1f}, {mbgr[2]:.1f}]"
                    )

            # Định kỳ 5 giây kiểm tra cấu hình động
            now = time.time()
            if (now - self.last_config_check) > 5.0:
                self.last_config_check = now
                old_rot = self.rotation
                old_bitrate = self.bitrate
                self.load_config()
                if self.rotation != old_rot:
                    self.get_logger().info(f"Phát hiện thay đổi config: Rotation đổi từ {old_rot} -> {self.rotation}")
                if self.bitrate != old_bitrate:
                    self.get_logger().info(f"Phát hiện thay đổi config: Bitrate đổi từ {old_bitrate} -> {self.bitrate}")
                    self.init_writer()

        except Exception as e:
            self.get_logger().warn(f"Lỗi xử lý frame camera: {e}")

    def standby_timer(self):
        now = time.time()
        # Nếu mất frame quá 1.5 giây, phát Standby Pattern & kích hoạt Watchdog
        if (now - self.last_frame_time) > 1.5:
            # Watchdog: nếu mất frame quá 3 giây, kiểm tra và thử re-init writer
            if (now - self.last_frame_time) > 3.0 and (now - self.last_reinit_attempt) > 3.0:
                self.init_writer()

            img = np.full((self.height, self.width, 3), (35, 30, 45), dtype=np.uint8)
            cv2.rectangle(img, (10, 10), (self.width - 10, self.height - 10), (0, 200, 255), 2)
            cv2.putText(img, "DRONE REALSENSE D435i - STANDBY", (30, 80),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 200), 2)
            timestr = time.strftime("%Y-%m-%d %H:%M:%S")
            cv2.putText(img, f"Time: {timestr}", (30, 150),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)
            cv2.putText(img, "URL : rtsp://192.168.144.141:8554/camera", (30, 200),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (100, 220, 255), 1)
            cv2.putText(img, f"Rotation: {self.rotation}", (30, 250),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (200, 200, 100), 1)

            t = time.time()
            bar_pos = int((np.sin(t * 3) + 1) * 0.5 * (self.width - 80)) + 40
            cv2.line(img, (bar_pos, 300), (bar_pos, 340), (0, 165, 255), 4)

            self.write_frame_safe(img)

    def destroy_node(self):
        if self.writer:
            try:
                self.writer.release()
            except Exception:
                pass
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = RTSPStreamer()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
