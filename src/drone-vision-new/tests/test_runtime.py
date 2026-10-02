import copy
import sys
import tempfile
import time
from pathlib import Path
import unittest
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from configuration import load_config, validate_config
from mavlink_client import Outbox, MavlinkClient
from yolo_streamer import LatestFrame, DetectionFrame, render_output, input_pipeline, output_pipeline
from target_selection import Detection

ROOT = Path(__file__).resolve().parents[1]

class RuntimeTests(unittest.TestCase):
    def config(self):
        config = load_config(ROOT / 'config/drone.yaml', check_model=False)
        return copy.deepcopy(config)

    def test_config_and_identity_validation(self):
        config = self.config()
        self.assertEqual(config['mavlink']['router_port'], 5760)
        self.assertEqual(config['mavlink']['component_id'], 192)
        self.assertEqual(config['mavlink']['camera_component_id'], 105)
        for key, value in [('router_port', 0), ('system_id', 0), ('component_id', 256), ('router_host', 'bad')]:
            bad = copy.deepcopy(config); bad['mavlink'][key] = value
            with self.assertRaises(ValueError): validate_config(bad, check_model=False)
        bad = copy.deepcopy(config); bad['yolo']['width'] = 641
        with self.assertRaises(ValueError): validate_config(bad, check_model=False)
        bad = copy.deepcopy(config); bad['yolo']['stale_seconds'] = float('nan')
        with self.assertRaises(ValueError): validate_config(bad, check_model=False)

    def test_camera_alias_and_rate_validation(self):
        config = self.config()
        for value in (99, 106, True, 105.0):
            bad = copy.deepcopy(config); bad['mavlink']['camera_component_id'] = value
            with self.assertRaises(ValueError): validate_config(bad, check_model=False)
        for value in (0, 11, float('nan'), float('inf')):
            bad = copy.deepcopy(config); bad['mavlink']['tracking_status_max_hz'] = value
            with self.assertRaises(ValueError): validate_config(bad, check_model=False)
        bad = copy.deepcopy(config); bad['mavlink']['component_id'] = 105
        with self.assertRaises(ValueError): validate_config(bad, check_model=False)

    def test_stop_clears_rendered_cached_inference_and_latest_status(self):
        client = MavlinkClient(self.config()['mavlink'])
        client.connected = True
        item = Detection((40, 40, 60, 60), .9, 'item')
        class Frame:
            shape = (100, 100, 3)
        cached = DetectionFrame(Frame(), [item])
        client.selection.set_point(.5, .5, .1, generation=3)
        with patch('yolo_streamer.render', side_effect=lambda frame, detections, selected: selected):
            self.assertEqual(render_output(cached, client), item)
            self.assertEqual(client.outbox.pop()['bbox'], [.4, .4, .6, .6])
            client._handle({'type': 'clear_selection', 'generation': 4})
            self.assertIsNone(render_output(cached, client))
            self.assertIsNone(client.outbox.pop()['bbox'])

    def test_selection_mailbox_does_not_replace_telemetry(self):
        box = Outbox(capacity=1)
        for i in range(100):
            box.put({'type': 'selection_state', 'generation': i})
            box.put({'type': 'telemetry', 'count': i})
        self.assertEqual(box.pop()['generation'], 99)
        self.assertEqual(box.pop()['count'], 99)
        self.assertIsNone(box.pop())

    def test_model_validation(self):
        config = self.config(); config['yolo']['model'] = '/missing/model.pt'
        with self.assertRaises(FileNotFoundError): validate_config(config)

    def test_mailbox_latest_stale_and_empty(self):
        mailbox=LatestFrame(); self.assertIsNone(mailbox.get(2))
        for i in range(100): mailbox.put(i)
        self.assertEqual(mailbox.get(2)[2], 99)
        mailbox.put('stale', timestamp=time.monotonic()-10)
        self.assertIsNone(mailbox.get(2))

    def test_ipc_queue_is_bounded_and_latest_only(self):
        outbox=Outbox(capacity=2)
        for i in range(100): self.assertTrue(outbox.put({'type':'telemetry', 'count':i}))
        self.assertEqual(outbox.pop()['count'], 99)
        self.assertTrue(outbox.put({'type':'get', 'request_id':1}))
        self.assertTrue(outbox.put({'type':'get', 'request_id':2}))
        self.assertFalse(outbox.put({'type':'get', 'request_id':3}))
        self.assertEqual(outbox.pop()['request_id'], 1)
        outbox.clear(); self.assertIsNone(outbox.pop())

    def test_stream_pipeline_bounds_and_endpoints(self):
        config=self.config()['yolo']
        self.assertIn('protocols=tcp', input_pipeline(config))
        self.assertIn('appsink max-buffers=1 drop=true', input_pipeline(config))
        self.assertIn('block=false max-buffers=2', output_pipeline(config))
        self.assertIn('leaky=downstream', output_pipeline(config))
        self.assertIn('1935/yolo', output_pipeline(config))

if __name__ == '__main__': unittest.main()
