"""Independent capture, inference and RTMP publish workers."""
import logging
from dataclasses import dataclass
from pathlib import Path
import threading
import time
from bbox_renderer import render
from target_selection import Detection

LOG = logging.getLogger('vision')

@dataclass(frozen=True)
class DetectionFrame:
    frame: object
    detections: list

def render_output(value, client):
    # Re-evaluate every published frame, including repeated inference output.
    # STOP/expiry therefore clears the rendered highlight without another YOLO run.
    if isinstance(value, DetectionFrame):
        height, width = value.frame.shape[:2]
        selected = client.select(value.detections, (width, height))
        return render(value.frame, value.detections, selected)
    height, width = value.shape[:2]
    client.select([], (width, height))
    return value

class LatestFrame:
    def __init__(self):
        self.lock = threading.Lock()
        self.sequence = 0
        self.value = None

    def put(self, frame, timestamp=None):
        with self.lock:
            self.sequence += 1
            self.value = (self.sequence, time.monotonic() if timestamp is None else timestamp, frame)

    def get(self, max_age):
        with self.lock:
            value = self.value
        return value if value and time.monotonic() - value[1] <= max_age else None

def quote(value):
    return '"' + str(value).replace('\\', '\\\\').replace('"', '\\"') + '"'

def input_pipeline(c):
    return (f'rtspsrc location={quote(c["input_url"])} protocols=tcp latency=100 '
            'drop-on-latency=true tcp-timeout=5000000 ! rtph264depay ! h264parse ! '
            'avdec_h264 max-threads=1 ! queue max-size-buffers=2 max-size-time=0 '
            'max-size-bytes=0 leaky=downstream ! videoconvert ! videoscale ! '
            f'video/x-raw,format=BGR,width={c["width"]},height={c["height"]} ! '
            'appsink max-buffers=1 drop=true sync=false')

def output_pipeline(c):
    return ('appsrc name=frames is-live=true format=time block=false max-buffers=2 '
            'leaky-type=downstream ! queue max-size-buffers=2 max-size-time=0 '
            'max-size-bytes=0 leaky=downstream ! videoconvert ! video/x-raw,format=I420 ! '
            f'x264enc tune=zerolatency speed-preset=ultrafast threads=1 bitrate={c["bitrate_kbps"]} '
            f'key-int-max={c["fps"]} bframes=0 byte-stream=false ! video/x-h264,profile=baseline ! '
            'h264parse config-interval=-1 ! flvmux streamable=true ! '
            f'rtmpsink location={quote(c["output_url"])} sync=false async=false')

def capture_loop(c, frames, stop):
    import cv2
    while not stop.is_set():
        capture = None
        try:
            capture = cv2.VideoCapture(input_pipeline(c), cv2.CAP_GSTREAMER)
            if not capture.isOpened():
                raise RuntimeError('RTSP capture did not open')
            LOG.info('Camera input connected: %s', c['input_url'])
            while not stop.is_set():
                ok, frame = capture.read()
                if not ok:
                    raise RuntimeError('RTSP frame read failed')
                frames.put(frame)
        except Exception as error:
            LOG.warning('Camera input reconnecting: %s', error)
        finally:
            if capture is not None:
                capture.release()
        stop.wait(c['reconnect_seconds'])

def is_camera_standby():
    import json
    try:
        return json.loads(Path('/run/drone/camera_stats.json').read_text()).get('is_standby', False)
    except (OSError, ValueError):
        return False

def inference_loop(c, model, frames, annotated, client, stop):
    sequence = -1
    report = 0
    while not stop.is_set():
        started = time.monotonic()
        item = frames.get(c['stale_seconds'])
        if item is None or item[0] == sequence:
            stop.wait(.02)
            continue
        sequence, captured, frame = item
        try:
            if is_camera_standby():
                import cv2
                output = frame.copy()
                cv2.putText(output, '[ STANDBY: YOLO INFERENCE PAUSED ]', (20, 35),
                            cv2.FONT_HERSHEY_SIMPLEX, .65, (0, 255, 255), 2)
                annotated.put(output, timestamp=captured)
                stop.wait(1 / c['inference_fps'])
                continue
            result = model.predict(frame, imgsz=c['imgsz'], conf=c['confidence'], device='cpu', verbose=False)[0]
            detections = []
            for box in result.boxes:
                label = result.names[int(box.cls[0])]
                detections.append(Detection(tuple(box.xyxy[0].tolist()), float(box.conf[0]), label))
            height, width = frame.shape[:2]
            annotated.put(DetectionFrame(frame, detections), timestamp=captured)
            duration = max(.001, time.monotonic() - started)
            if started - report >= 1:
                # Latest-only publication enqueues data; no socket or pipe operation occurs here.
                client.telemetry(confidence=c['confidence'], inference_fps=min(c['inference_fps'], 1 / duration),
                                 width=width, height=height, fps=c['fps'], count=min(255, len(detections)),
                                 model=Path(c['model']).name)
                report = started
        except Exception:
            LOG.exception('Inference failed')
            stop.wait(c['reconnect_seconds'])
        stop.wait(max(0, 1 / c['inference_fps'] - (time.monotonic() - started)))

def publish_loop(c, Gst, annotated, client, stop):
    while not stop.is_set():
        pipeline = None
        try:
            pipeline = Gst.parse_launch(output_pipeline(c))
            source = pipeline.get_by_name('frames')
            source.set_property('caps', Gst.Caps.from_string(
                f'video/x-raw,format=BGR,width={c["width"]},height={c["height"]},framerate={c["fps"]}/1'))
            if pipeline.set_state(Gst.State.PLAYING) == Gst.StateChangeReturn.FAILURE:
                raise RuntimeError('RTMP pipeline did not start')
            bus = pipeline.get_bus()
            epoch = time.monotonic()
            while not stop.is_set():
                started = time.monotonic()
                error = bus.timed_pop_filtered(0, Gst.MessageType.ERROR | Gst.MessageType.EOS)
                if error:
                    raise RuntimeError(str(error.parse_error()) if error.type == Gst.MessageType.ERROR else 'EOS')
                item = annotated.get(c['stale_seconds'])
                if item:
                    data = render_output(item[2], client).tobytes()
                    buffer = Gst.Buffer.new_allocate(None, len(data), None)
                    buffer.fill(0, data)
                    buffer.pts = buffer.dts = int((started - epoch) * Gst.SECOND)
                    buffer.duration = Gst.SECOND // c['fps']
                    if source.emit('push-buffer', buffer) != Gst.FlowReturn.OK:
                        raise RuntimeError('RTMP output rejected frame')
                else:
                    client.select([], (c['width'], c['height']))
                stop.wait(max(0, 1 / c['fps'] - (time.monotonic() - started)))
        except Exception as error:
            LOG.warning('RTMP output reconnecting: %s', error)
        finally:
            if pipeline is not None:
                pipeline.set_state(Gst.State.NULL)
        stop.wait(c['reconnect_seconds'])
