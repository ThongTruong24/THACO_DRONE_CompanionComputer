import importlib.util
from pathlib import Path
import tempfile
import time
import unittest
from unittest.mock import Mock, patch
import yaml

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('worker', ROOT / 'scripts/yolo_streamer.py')
worker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(worker)


class WorkerTests(unittest.TestCase):
    def test_latest_only(self):
        box = worker.LatestFrame()
        for i in range(100):
            box.put(i)
        self.assertEqual(box.get(2)[2], 99)
        self.assertEqual(box.sequence, 100)

    def test_stale_discarded(self):
        box = worker.LatestFrame()
        box.put('old', time.monotonic() - 10)
        self.assertIsNone(box.get(2))

    def test_empty(self):
        self.assertIsNone(worker.LatestFrame().get(2))

    def test_config(self):
        cfg = yaml.safe_load((ROOT / 'drone.yaml').read_text())
        cfg['yolo']['model'] = str(ROOT / 'yolo26n.pt')
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'config.yaml'
            path.write_text(yaml.safe_dump(cfg))
            self.assertEqual(worker.load_config(path)['fps'], 10)
            cfg['yolo']['width'] = 641
            path.write_text(yaml.safe_dump(cfg))
            with self.assertRaises(ValueError):
                worker.load_config(path)

    def test_model_missing(self):
        with patch.object(worker.Path, 'is_file', return_value=False):
            with self.assertRaises(FileNotFoundError):
                worker.load_config(ROOT / 'drone.yaml')

    def test_pipeline_bounded(self):
        c = yaml.safe_load((ROOT / 'drone.yaml').read_text())['yolo']
        self.assertIn('appsink max-buffers=1 drop=true', worker.input_pipeline(c))
        self.assertIn('block=false max-buffers=2', worker.output_pipeline(c))
        self.assertIn('leaky=downstream', worker.output_pipeline(c))

    def test_capture_failure_releases(self):
        stop = Mock()
        stop.is_set.side_effect = [False, True]
        cv = Mock()
        cv.VideoCapture.return_value.isOpened.return_value = False
        c = yaml.safe_load((ROOT / 'drone.yaml').read_text())['yolo']
        with self.assertLogs('yolo', level='ERROR'):
            worker.capture_loop(c, cv, worker.LatestFrame(), stop)
        cv.VideoCapture.return_value.release.assert_called_once()
        stop.wait.assert_called_once_with(1)

    def test_stopped_worker_does_not_infer(self):
        stop = Mock()
        stop.is_set.return_value = True
        model = Mock()
        worker.inference_loop({}, model, None, None, stop)
        model.predict.assert_not_called()

    def test_no_mavlink_or_usb(self):
        source = (ROOT / 'scripts/yolo_streamer.py').read_text()
        self.assertNotIn('import pymavlink', source)
        self.assertNotIn('/dev/video', source)


if __name__ == '__main__':
    unittest.main()
