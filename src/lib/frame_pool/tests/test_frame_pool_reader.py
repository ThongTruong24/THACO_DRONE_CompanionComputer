#!/usr/bin/env python3
import os
import shutil
import subprocess
import tempfile
import unittest
import numpy as np

from frame_pool import FramePoolReader


class TestFramePoolCrossLanguage(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.mkdtemp(prefix="pool_test_py_")

    def tearDown(self):
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_python_reads_cpp_written_pool_and_overwritten_returns_none(self):
        helper_bin = os.environ.get("FRAME_POOL_HELPER_BIN")
        if not helper_bin or not os.path.exists(helper_bin):
            # Try finding it in relative build directories
            candidates = [
                os.path.join(os.path.dirname(__file__), "../../../build/cc_frame_pool/frame_pool_writer_helper"),
                "./build/cc_frame_pool/frame_pool_writer_helper",
            ]
            for c in candidates:
                if os.path.exists(c):
                    helper_bin = c
                    break

        self.assertIsNotNone(helper_bin, "frame_pool_writer_helper binary not found")

        # Run C++ writer helper: writes 5 frames into 3 slots (slot 0 overwritten at frame 4, slot 1 at frame 5)
        res = subprocess.run(
            [helper_bin, "--dir", self.test_dir, "--count", "5"],
            capture_output=True,
            text=True,
            check=True
        )

        lines = res.stdout.strip().split("\n")
        self.assertEqual(len(lines), 5)
        descs = []
        for line in lines:
            parts = line.strip().split()
            descs.append((int(parts[0]), int(parts[1]), int(parts[2])))

        reader = FramePoolReader(self.test_dir)

        # Frame 1: slot 0, seq 2. But frame 4 was also written to slot 0 (seq 8)!
        gen1, slot1, seq1 = descs[0]
        # Slot 0 has been overwritten by frame 4! Reading old seq1 MUST return None
        old_frame = reader.read_frame(gen1, slot1, seq1)
        self.assertIsNone(old_frame, "Reading overwritten slot must return None")

        # Latest frame for slot 0 is frame 4 (descs[3]):
        gen4, slot4, seq4 = descs[3]
        f4 = reader.read_frame(gen4, slot4, seq4)
        self.assertIsNotNone(f4)
        ts, color, depth = f4
        self.assertEqual(ts, 4000)
        self.assertEqual(color.shape, (48, 64, 3))
        self.assertTrue(np.all(color == 4))
        self.assertEqual(depth.shape, (48, 64))
        self.assertTrue(np.all(depth == 8)) # 4 * 2

        reader.close()


if __name__ == "__main__":
    unittest.main()
