"""Run inside the final image with a test-only MediaMTX binary at /tmp/mediamtx.

Exercises the packaged app with synthetic RTSP input, real YOLO inference and
RTMP publication. No MAVLink router runs: TCP reconnect must leave Vision working.
"""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import yaml
sys.path.insert(0, '/app/scripts')
import cv2
import numpy as np
from bbox_renderer import render
from target_selection import Detection
from mavlink_client import MavlinkClient
from yolo_streamer import DetectionFrame, render_output

def run():
    processes = []
    with tempfile.TemporaryDirectory(prefix='vision-smoke-') as directory:
        root = Path(directory)
        (root / 'mediamtx.yml').write_text('logLevel: warn\nrtspAddress: :8554\nrtmpAddress: :1935\nwebrtc: no\nhls: no\nsrt: no\npaths:\n  camera:\n    overridePublisher: false\n  yolo:\n    overridePublisher: false\n')
        config = yaml.safe_load(Path('/app/config/drone.yaml').read_text())
        # QEMU throughput is not Pi throughput. Keep the real model and network
        # path, but use a small test frame and allow slow emulated inference.
        config['yolo'].update(width=320, height=240, imgsz=160, fps=5,
                              inference_fps=1, stale_seconds=60)
        (root / 'vision.yml').write_text(yaml.safe_dump(config))
        logs = []
        try:
            for name, command in [
                ('mediamtx', ['/tmp/mediamtx', str(root / 'mediamtx.yml')]),
                ('camera', ['gst-launch-1.0', '-q', 'videotestsrc', 'is-live=true', 'pattern=ball', '!',
                            'video/x-raw,width=640,height=360,framerate=2/1', '!', 'videoconvert', '!',
                            'x264enc', 'tune=zerolatency', 'speed-preset=ultrafast', 'key-int-max=2', '!',
                            'h264parse', '!', 'flvmux', 'streamable=true', '!', 'rtmpsink', 'location=rtmp://127.0.0.1:1935/camera']),
                ('vision', ['/app/entrypoint.sh']),
            ]:
                log = open(root / f'{name}.log', 'w'); logs.append(log)
                environment = dict(os.environ, VISION_CONFIG=str(root / 'vision.yml'))
                processes.append(subprocess.Popen(command, env=environment, stdout=log, stderr=subprocess.STDOUT))
                time.sleep(1)
            deadline = time.monotonic() + int(os.getenv('SMOKE_TIMEOUT_SECONDS', '300'))
            seen = []
            while time.monotonic() < deadline:
                if any(p.poll() is not None for p in processes):
                    raise RuntimeError('smoke fixture/application exited')
                capture = cv2.VideoCapture(
                    'rtspsrc location=rtsp://127.0.0.1:8554/yolo protocols=tcp latency=100 tcp-timeout=3000000 ! '
                    'rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=BGR ! '
                    'appsink max-buffers=1 drop=true sync=false', cv2.CAP_GSTREAMER)
                try:
                    if capture.isOpened():
                        for _ in range(3):
                            ok, frame = capture.read()
                            if ok:
                                assert frame.shape == (240, 320, 3)
                                seen.append(frame)
                    if len(seen) >= 3:
                        break
                finally:
                    capture.release()
                time.sleep(1)
            assert len(seen) >= 3, 'no decoded /yolo frames from real inference pipeline'
            raw = cv2.VideoCapture(
                'rtspsrc location=rtsp://127.0.0.1:8554/camera protocols=tcp latency=100 tcp-timeout=3000000 ! '
                'rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=BGR ! '
                'appsink max-buffers=1 drop=true sync=false', cv2.CAP_GSTREAMER)
            try:
                ok, frame = raw.read()
                assert ok and frame.shape == (360, 640, 3), 'raw /camera must remain readable'
            finally:
                raw.release()
            # Real OpenCV pixels independently prove renderer styles, even when the
            # deterministic synthetic source contains no recognized YOLO objects.
            image = np.zeros((100, 100, 3), dtype=np.uint8)
            selected = Detection((10, 30, 40, 60), .9, 'selected')
            normal = Detection((60, 30, 90, 60), .8, 'normal')
            output = render(image, [selected, normal], selected)
            assert tuple(output[45, 10]) == (0, 255, 255)
            assert tuple(output[45, 60]) == (0, 255, 0)
            assert not image.any(), 'render must preserve the input frame'
            client = MavlinkClient(config['mavlink'])
            cached = DetectionFrame(image, [selected, normal])
            client.selection.set_point(.25, .45, .1)
            assert tuple(render_output(cached, client)[45, 10]) == (0, 255, 255)
            client.selection.clear()
            assert tuple(render_output(cached, client)[45, 10]) == (0, 255, 0), 'STOP must repaint cached detection frame'
            client.selection.set_point(.25, .45, .1, now=time.monotonic()-10)
            assert tuple(render_output(cached, client)[45, 10]) == (0, 255, 0), 'expired selection must repaint cached frame'
            print('PASS real YOLO at test resolution 320x240/imgsz160: /camera -> inference -> RTMP /yolo -> decoded RTSP /yolo; raw 640x360 /camera readable; green/yellow renderer; router absent without inference blocking', flush=True)
        except Exception:
            for log in logs: log.flush()
            for path in root.glob('*.log'): print(path.read_text(), file=sys.stderr)
            raise
        finally:
            for process in reversed(processes):
                process.terminate()
                try: process.wait(timeout=15)
                except subprocess.TimeoutExpired: process.kill(); process.wait()
            for log in logs: log.close()

if __name__ == '__main__': run()
