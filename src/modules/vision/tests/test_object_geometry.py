import importlib.util
import sys
from pathlib import Path
import unittest

try:
    from vision import object_geometry as geometry
except ImportError:
    ROOT = Path(__file__).resolve().parents[1]
    spec = importlib.util.spec_from_file_location('geometry', ROOT / 'vision/object_geometry.py')
    geometry = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = geometry
    spec.loader.exec_module(geometry)


class ObjectGeometryTests(unittest.TestCase):
    def test_deprojects_rectified_pixel(self):
        intrinsics = geometry.Intrinsics(fx=100, fy=100, cx=50, cy=40)
        self.assertEqual(geometry.deproject_pixel(70, 30, 2.0, intrinsics), (0.4, -0.2, 2.0))

    def test_roi_depth_uses_median_and_rejects_outlier(self):
        depth = [[2.0 for _ in range(20)] for _ in range(20)]
        depth[10][10] = 9.0
        estimate = geometry.estimate_roi_depth(depth, (2, 2, 18, 18), 0.3, 10.0)
        self.assertIsNotNone(estimate)
        self.assertEqual(estimate.range_m, 2.0)
        self.assertGreater(estimate.valid_ratio, 0.9)

    def test_roi_depth_rejects_insufficient_valid_samples(self):
        depth = [[0.0 for _ in range(10)] for _ in range(10)]
        self.assertIsNone(geometry.estimate_roi_depth(depth, (0, 0, 10, 10), 0.3, 10.0))


if __name__ == '__main__':
    unittest.main()
