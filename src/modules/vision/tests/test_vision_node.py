"""Unit tests for VisionNode, detector, publisher, and frame pool integration."""
import os
import shutil
import struct
import tempfile
import time
import unittest
import numpy as np

from vision.detector import Detector, sample_depth
from vision.publisher import VisionPublisher
import sys

try:
    from frame_pool import FramePoolReader, MAGIC, VERSION, FILE_HDR_SIZE, SLOT_HDR_SIZE
except ImportError:
    _fp_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../lib/frame_pool"))
    if _fp_path not in sys.path:
        sys.path.insert(0, _fp_path)
    from frame_pool import FramePoolReader, MAGIC, VERSION, FILE_HDR_SIZE, SLOT_HDR_SIZE


class VisionComponentsTest(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.mkdtemp(prefix="test_vision_")

    def tearDown(self):
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_detector_fallback_without_weights(self):
        """Detector works in fallback mode if weights file does not exist."""
        det = Detector("/nonexistent/model.pt", imgsz=320, confidence=0.35)
        color = np.zeros((480, 640, 3), dtype=np.uint8)
        annotated, count = det.detect(color)
        self.assertEqual(count, 0)
        self.assertEqual(annotated.shape, color.shape)

    def test_sample_depth(self):
        """Depth sampling calculates median correctly within range."""
        depth = np.full((100, 100), 2000, dtype=np.uint16)  # 2000 mm = 2.0 m
        depth[50, 50] = 50000  # outlier
        dist = sample_depth(depth, cx=50, cy=50, radius=5, min_m=0.3, max_m=10.0, depth_scale=0.001)
        self.assertIsNotNone(dist)
        self.assertAlmostEqual(dist, 2.0, places=2)

    def test_read_from_frame_pool(self):
        """Verify FramePoolReader correctly reads valid frame and detects torn frame."""
        generation = 1
        width = 32
        height = 24
        slot_size = SLOT_HDR_SIZE + (width * height * 3) + (width * height * 2)
        total_size = FILE_HDR_SIZE + slot_size

        pool_path = os.path.join(self.test_dir, f"pool.{generation}")
        with open(pool_path, "wb") as f:
            # File header
            enc_color = b"BGR8\x00\x00\x00\x00"
            enc_depth = b"Z16\x00\x00\x00\x00\x00"
            hdr = struct.pack(
                "<4sIIII8sBBBB8sfII12s",
                MAGIC, VERSION, generation, width, height,
                enc_color, 1, 0, 0, 0, enc_depth, 0.001,
                slot_size, 1, b"\x00" * 12
            )
            f.write(hdr)

            # Slot header: seq=2 (even -> valid), ts=999999
            color_size = width * height * 3
            depth_size = width * height * 2
            slot_hdr = struct.pack(
                "<QQIIII32s",
                2, 999999, SLOT_HDR_SIZE, color_size, SLOT_HDR_SIZE + color_size, depth_size, b"\x00" * 32
            )
            f.write(slot_hdr)
            # Color data (all 42)
            f.write(b"\x2a" * color_size)
            # Depth data (uint16 value 1500)
            f.write(struct.pack("<H", 1500) * (width * height))

        reader = FramePoolReader(self.test_dir)
        res = reader.read_frame(generation=1, slot=0, seq=2)
        self.assertIsNotNone(res)
        ts, color, depth = res
        self.assertEqual(ts, 999999)
        self.assertEqual(color.shape, (height, width, 3))
        self.assertEqual(color[0, 0, 0], 42)
        self.assertIsNotNone(depth)
        self.assertEqual(depth.shape, (height, width))
        self.assertEqual(depth[0, 0], 1500)

        # Torn frame: odd sequence number 3
        res_torn = reader.read_frame(generation=1, slot=0, seq=3)
        self.assertIsNone(res_torn)
        reader.close()

    def test_no_encode_when_no_frame(self):
        """Publisher must not encode frames when no frames are pushed."""
        pub = VisionPublisher(width=320, height=240, fps=15, stale_seconds=1.0)
        self.assertTrue(pub.is_idle)
        self.assertEqual(pub.published_frames, 0)
        self.assertTrue(pub.check_stale())
        pub.stop_pipeline()

    def test_idle_after_stale(self):
        """Publisher transitions to idle and NULL pipeline after stale_seconds."""
        pub = VisionPublisher(width=320, height=240, fps=15, stale_seconds=0.1)
        pub.is_idle = False
        pub.last_frame_time = time.monotonic() - 1.0  # 1 sec ago > 0.1s
        is_stale = pub.check_stale()
        self.assertTrue(is_stale)
        self.assertTrue(pub.is_idle)
        pub.stop_pipeline()


if __name__ == "__main__":
    unittest.main()
