def is_camera_standby():
    try:
        import json
        with open('/run/drone/camera_stats.json', 'r') as f:
            data = json.load(f)
            return data.get('is_standby', False)
    except Exception:
        return False

#!/usr/bin/env python3
"""YOLO RTSP/RealSense worker with Depth Estimation.

Modes:
  input_source=realsense  - Read color+depth directly from RealSense D435i via pyrealsense2.
                            Annotates each bounding box with real distance (m).
  input_source=rtsp       - Fallback: read RGB from RTSP, depth disabled.
"""
import logging
import os
from pathlib import Path
import signal
import threading
import time
import numpy as np
import yaml

LOG = logging.getLogger('yolo')


# ──────────────────────────────────────────────────────────────────────────────
# CONFIG
# ──────────────────────────────────────────────────────────────────────────────

def load_config(path):
    with open(path, encoding='utf-8') as handle:
        raw = yaml.safe_load(handle)
    cfg = raw['yolo']
    for key in ('width', 'height', 'fps', 'imgsz', 'threads', 'bitrate_kbps',
                'inference_fps', 'stale_seconds', 'reconnect_seconds'):
        if not isinstance(cfg[key], (int, float)) or cfg[key] <= 0:
            raise ValueError(f'yolo.{key} must be positive')
    if cfg['width'] % 2 or cfg['height'] % 2:
        raise ValueError('H.264 dimensions must be even')
    if not 0 < cfg['confidence'] <= 1:
        raise ValueError('Invalid confidence')
    if not Path(cfg['model']).is_file():
        raise FileNotFoundError(cfg['model'])
    # input_source default = rtsp for backward compat
    cfg.setdefault('input_source', 'rtsp')
    if cfg['input_source'] == 'rtsp':
        if not cfg.get('input_url', '').startswith('rtsp://'):
            raise ValueError('Input must be RTSP when input_source=rtsp')
    if not cfg['output_url'].startswith('rtmp://127.0.0.1:'):
        raise ValueError('Output must target local MediaMTX')

    depth_cfg = raw.get('depth', {})
    depth_cfg.setdefault('enabled', False)
    depth_cfg.setdefault('width', 640)
    depth_cfg.setdefault('height', 480)
    depth_cfg.setdefault('fps', 6)
    depth_cfg.setdefault('min_distance_m', 0.3)
    depth_cfg.setdefault('max_distance_m', 10.0)
    depth_cfg.setdefault('sample_radius', 5)
    depth_cfg.setdefault('mavlink_publish', False)
    depth_cfg.setdefault('unit', 'm')
    return cfg, depth_cfg


# ──────────────────────────────────────────────────────────────────────────────
# THREAD-SAFE FRAME MAILBOX
# ──────────────────────────────────────────────────────────────────────────────

class LatestFrame:
    """Single-slot mailbox – prevents accumulated latency."""
    def __init__(self):
        self.lock = threading.Lock()
        self.value = None
        self.sequence = 0

    def put(self, frame, depth=None, timestamp=None):
        with self.lock:
            self.sequence += 1
            self.value = (self.sequence,
                          time.monotonic() if timestamp is None else timestamp,
                          frame, depth)

    def get(self, max_age):
        with self.lock:
            value = self.value
        return value if value and time.monotonic() - value[1] <= max_age else None


# ──────────────────────────────────────────────────────────────────────────────
# DEPTH HELPER
# ──────────────────────────────────────────────────────────────────────────────

def sample_depth(depth_frame_np, cx, cy, radius, min_m, max_m, depth_scale):
    """Return median depth (m) in a radius x radius window centred on (cx, cy).

    depth_frame_np  : uint16 ndarray, raw Z16 values from RealSense
    depth_scale     : float, metres per unit (typically 0.001 for D435i)
    Returns None when all sampled pixels are outside [min_m, max_m].
    """
    h, w = depth_frame_np.shape
    x0, x1 = max(0, cx - radius), min(w, cx + radius + 1)
    y0, y1 = max(0, cy - radius), min(h, cy + radius + 1)
    patch = depth_frame_np[y0:y1, x0:x1].astype(np.float32) * depth_scale
    valid = patch[(patch >= min_m) & (patch <= max_m)]
    if valid.size == 0:
        return None
    return float(np.median(valid))


# ──────────────────────────────────────────────────────────────────────────────
# FRAME SOURCES
# ──────────────────────────────────────────────────────────────────────────────

class RealSenseDepthSource:
    """Reads aligned Color + Depth frames directly from RealSense D435i."""

    def __init__(self, c, d, stop):
        self.c = c
        self.d = d
        self.stop = stop

    def run(self, mailbox):
        import pyrealsense2 as rs
        import cv2 as cv

        pipeline = rs.pipeline()
        rs_cfg = rs.config()

        color_w, color_h = self.c['width'], self.c['height']
        depth_w = self.d['width']
        depth_h = self.d['height']
        depth_fps = self.d['fps']

        rs_cfg.enable_stream(rs.stream.color, color_w, color_h, rs.format.bgr8, self.c['fps'])
        rs_cfg.enable_stream(rs.stream.depth, depth_w, depth_h, rs.format.z16, depth_fps)

        align = rs.align(rs.stream.color)

        while not self.stop.is_set():
            profile = None
            try:
                profile = pipeline.start(rs_cfg)
                depth_sensor = profile.get_device().first_depth_sensor()
                depth_scale = depth_sensor.get_depth_scale()
                LOG.info('[RealSense] depth_scale=%.6f m/unit', depth_scale)

                while not self.stop.is_set():
                    frames = pipeline.wait_for_frames(timeout_ms=2000)
                    aligned = align.process(frames)
                    color_frame = aligned.get_color_frame()
                    depth_frame = aligned.get_depth_frame()
                    if not color_frame or not depth_frame:
                        continue

                    color_np = np.asanyarray(color_frame.get_data())
                    depth_np = np.asanyarray(depth_frame.get_data())  # uint16 Z16

                    # Resize color to YOLO input size if needed
                    if color_np.shape[1] != color_w or color_np.shape[0] != color_h:
                        color_np = cv.resize(color_np, (color_w, color_h))

                    mailbox.put(color_np, depth=(depth_np, depth_scale))

            except Exception:
                LOG.exception('[RealSense] Capture error; retrying in 2s')
            finally:
                try:
                    pipeline.stop()
                except Exception:
                    pass
            self.stop.wait(self.c['reconnect_seconds'])


class RtspSource:
    """Fallback: reads RGB from RTSP stream (no depth)."""

    def __init__(self, c, stop):
        self.c = c
        self.stop = stop

    def run(self, mailbox):
        import cv2 as cv
        while not self.stop.is_set():
            cap = None
            try:
                pipeline_str = (
                    f'rtspsrc location="{self.c["input_url"]}" protocols=tcp latency=100 '
                    'drop-on-latency=true tcp-timeout=5000000 ! rtph264depay ! h264parse ! '
                    'avdec_h264 max-threads=1 ! queue max-size-buffers=2 max-size-time=0 '
                    'max-size-bytes=0 leaky=downstream ! videoconvert ! videoscale ! '
                    f'video/x-raw,format=BGR,width={self.c["width"]},height={self.c["height"]} ! '
                    'appsink max-buffers=1 drop=true sync=false'
                )
                cap = cv.VideoCapture(pipeline_str, cv.CAP_GSTREAMER)
                if not cap.isOpened():
                    raise RuntimeError('Cannot open RTSP stream')
                LOG.info('[RTSP] Connected: %s', self.c['input_url'])
                while not self.stop.is_set():
                    ok, frame = cap.read()
                    if not ok:
                        raise RuntimeError('RTSP read failed')
                    mailbox.put(frame, depth=None)
            except Exception as e:
                LOG.warning('[RTSP] Stream %s reconnecting: %s (will retry in %.1fs)',
                            self.c['input_url'], e, self.c['reconnect_seconds'])
            finally:
                if cap is not None:
                    cap.release()
            self.stop.wait(self.c['reconnect_seconds'])


# ──────────────────────────────────────────────────────────────────────────────
# INFERENCE LOOP
# ──────────────────────────────────────────────────────────────────────────────

def inference_loop(c, d, model, frames_box, annotated_box, stop):
    min_m = d['min_distance_m']
    max_m = d['max_distance_m']
    radius = d['sample_radius']
    depth_enabled = d['enabled'] and c['input_source'] == 'realsense'

    sequence = -1
    while not stop.is_set():
        started = time.monotonic()
        item = frames_box.get(c['stale_seconds'])
        if item is None or item[0] == sequence:
            stop.wait(0.02)
            continue
        sequence, captured, frame, depth_payload = item

        try:
            if is_camera_standby():
                import cv2 as cv
                annotated = frame.copy()
                cv.putText(annotated, "[ STANDBY: YOLO INFERENCE PAUSED ]", (30, 40),
                           cv.FONT_HERSHEY_SIMPLEX, 0.7, (0, 200, 255), 2, cv.LINE_AA)
                annotated_box.put(annotated, timestamp=captured)
                stop.wait(max(0, 1 / c['inference_fps'] - (time.monotonic() - started)))
                continue

            results = model.predict(frame, imgsz=c['imgsz'], conf=c['confidence'],
                                    device='cpu', verbose=False)
            annotated = results[0].plot()  # BGR numpy with bboxes

            if depth_enabled and depth_payload is not None:
                depth_np, depth_scale = depth_payload
                boxes = results[0].boxes
                for box in boxes:
                    # box.xyxy shape: [1, 4] tensor
                    x1, y1, x2, y2 = box.xyxy[0].tolist()
                    cx = int((x1 + x2) / 2)
                    cy = int((y1 + y2) / 2)

                    # Scale bbox centre to depth frame coordinates
                    scale_x = depth_np.shape[1] / frame.shape[1]
                    scale_y = depth_np.shape[0] / frame.shape[0]
                    dcx = int(cx * scale_x)
                    dcy = int(cy * scale_y)

                    dist = sample_depth(depth_np, dcx, dcy, radius,
                                        min_m, max_m, depth_scale)
                    if dist is not None:
                        label = f'{dist:.2f}m'
                        # Draw distance label above bbox centre on annotated frame
                        import cv2 as cv
                        cv.putText(annotated, label,
                                   (cx - 20, max(int(y1) - 8, 12)),
                                   cv.FONT_HERSHEY_SIMPLEX, 0.65,
                                   (0, 255, 255), 2, cv.LINE_AA)

            annotated_box.put(annotated, timestamp=captured)
            LOG.debug('Inference %.1f ms', (time.monotonic() - started) * 1000)

        except Exception:
            LOG.exception('Inference failed')
            stop.wait(c['reconnect_seconds'])

        stop.wait(max(0, 1 / c['inference_fps'] - (time.monotonic() - started)))


# ──────────────────────────────────────────────────────────────────────────────
# PUBLISH LOOP (GStreamer → RTMP → MediaMTX)
# ──────────────────────────────────────────────────────────────────────────────

def quote(value):
    return '"' + str(value).replace('\\', '\\\\').replace('"', '\\"')+  '"'


def output_pipeline(c):
    return (
        'appsrc name=frames is-live=true format=time block=false max-buffers=2 '
        'leaky-type=downstream ! queue max-size-buffers=2 max-size-time=0 '
        'max-size-bytes=0 leaky=downstream ! videoconvert ! video/x-raw,format=I420 ! '
        f'x264enc tune=zerolatency speed-preset=ultrafast threads=1 bitrate={c["bitrate_kbps"]} '
        f'key-int-max={c["fps"]} bframes=0 byte-stream=false ! video/x-h264,profile=baseline ! '
        # Repeat codec parameters at each IDR so late RTSP readers can decode on the next GOP.
        f'h264parse config-interval=-1 ! flvmux streamable=true ! rtmpsink location={quote(c["output_url"])} '
        'sync=false async=false'
    )


def publish_loop(c, Gst, annotated_box, stop):
    while not stop.is_set():
        pipeline = None
        try:
            pipeline = Gst.parse_launch(output_pipeline(c))
            source = pipeline.get_by_name('frames')
            source.set_property('caps', Gst.Caps.from_string(
                f'video/x-raw,format=BGR,width={c["width"]},height={c["height"]},'
                f'framerate={c["fps"]}/1'))
            if pipeline.set_state(Gst.State.PLAYING) == Gst.StateChangeReturn.FAILURE:
                raise RuntimeError('Output pipeline failed to start')
            bus = pipeline.get_bus()
            epoch = time.monotonic()
            published, report = 0, epoch
            while not stop.is_set():
                started = time.monotonic()
                msg = bus.timed_pop_filtered(0, Gst.MessageType.ERROR | Gst.MessageType.EOS)
                if msg:
                    raise RuntimeError(
                        str(msg.parse_error()) if msg.type == Gst.MessageType.ERROR else 'EOS')
                item = annotated_box.get(c['stale_seconds'])
                if item:
                    data = item[2].tobytes()
                    buf = Gst.Buffer.new_allocate(None, len(data), None)
                    buf.fill(0, data)
                    buf.pts = int((started - epoch) * Gst.SECOND)
                    buf.dts = buf.pts
                    buf.duration = Gst.SECOND // c['fps']
                    if source.emit('push-buffer', buf) != Gst.FlowReturn.OK:
                        raise RuntimeError('Output rejected frame')
                    published += 1
                if started - report >= 10:
                    LOG.info('Output %.1f FPS', published / (started - report))
                    published, report = 0, started
                stop.wait(max(0, 1 / c['fps'] - (time.monotonic() - started)))
        except Exception:
            LOG.exception('Publisher disconnected; retrying')
        finally:
            if pipeline is not None:
                pipeline.set_state(Gst.State.NULL)
        stop.wait(c['reconnect_seconds'])


# ──────────────────────────────────────────────────────────────────────────────
# MAIN
# ──────────────────────────────────────────────────────────────────────────────

def main():
    logging.basicConfig(level=os.getenv('LOG_LEVEL', 'INFO'),
                        format='%(asctime)s %(levelname)s %(message)s')
    c, d = load_config(os.getenv('VISION_CONFIG', '/app/config/drone.yaml'))

    os.environ['OMP_NUM_THREADS'] = str(c['threads'])

    import gi
    gi.require_version('Gst', '1.0')
    from gi.repository import Gst
    from ultralytics import YOLO
    import torch

    torch.set_num_threads(c['threads'])
    torch.set_num_interop_threads(1)
    Gst.init(None)

    model = YOLO(c['model'], task='detect')

    stop = threading.Event()
    for sig in (signal.SIGINT, signal.SIGTERM):
        signal.signal(sig, lambda *_: stop.set())

    frames_box = LatestFrame()
    annotated_box = LatestFrame()

    # Choose frame source
    source_name = c['input_source']
    if source_name == 'realsense':
        try:
            import pyrealsense2  # noqa: F401 – validate importable before spawning thread
            source = RealSenseDepthSource(c, d, stop)
            LOG.info('[Main] Using RealSense D435i (color+depth)')
        except ImportError:
            LOG.warning('[Main] pyrealsense2 not available; falling back to RTSP (no depth)')
            c['input_source'] = 'rtsp'
            d['enabled'] = False
            source = RtspSource(c, stop)
    else:
        d['enabled'] = False
        source = RtspSource(c, stop)
        LOG.info('[Main] Using RTSP source (depth disabled)')

    workers = [
        threading.Thread(target=source.run, args=(frames_box,), daemon=True, name='capture'),
        threading.Thread(target=inference_loop, args=(c, d, model, frames_box, annotated_box, stop),
                         daemon=True, name='inference'),
    ]
    for w in workers:
        w.start()

    if d['enabled'] and c['input_source'] == 'realsense':
        LOG.info('[Main] YOLO ready with DEPTH: model=%s  output=%s', c['model'], c['output_url'])
    else:
        LOG.info('[Main] YOLO ready (no depth): model=%s  output=%s', c['model'], c['output_url'])

    try:
        publish_loop(c, Gst, annotated_box, stop)
    finally:
        stop.set()
        for w in workers:
            w.join(timeout=6)


if __name__ == '__main__':
    main()
