"""Bounded, nonblocking pipe client for the native MAVLink TCP worker."""
from collections import deque
import json
import logging
import os
import select
import subprocess
import threading
import time
from target_selection import Selection

LOG = logging.getLogger('mavlink')

class Outbox:
    def __init__(self, capacity=32):
        self.capacity = capacity
        self.reliable = deque()
        self.latest = None
        self.selection_state = None
        self.lock = threading.Lock()

    def put(self, event):
        with self.lock:
            if event['type'] == 'telemetry':
                self.latest = event
                return True
            if event['type'] == 'selection_state':
                self.selection_state = event
                return True
            if len(self.reliable) >= self.capacity:
                return False
            self.reliable.append(event)
            return True

    def pop(self):
        with self.lock:
            if self.reliable:
                return self.reliable.popleft()
            if self.selection_state is not None:
                event, self.selection_state = self.selection_state, None
                return event
            event, self.latest = self.latest, None
            return event

    def clear(self):
        with self.lock:
            self.reliable.clear()
            self.latest = None
            self.selection_state = None

class MavlinkClient:
    def __init__(self, config, selection_ttl=5, executable='/usr/local/bin/mavlink-bridge'):
        self.config = config
        self.executable = executable
        self.selection = Selection(selection_ttl)
        self.selection_ttl = selection_ttl
        self.outbox = Outbox()
        self.events = deque(maxlen=64)
        self.condition = threading.Condition()
        self.stop_event = threading.Event()
        self.connected = False
        self.process = None
        self.worker = None

    def command_line(self):
        c = self.config
        return [self.executable, '--host', c['router_host'], '--port', str(c['router_port']),
                '--system-id', str(c['system_id']), '--component-id', str(c['component_id']),
                '--camera-component-id', str(c.get('camera_component_id', 105)),
                '--status-max-hz', str(c.get('tracking_status_max_hz', 2)),
                '--selection-ttl-s', str(self.selection_ttl),
                '--selection-stale-s', str(c.get('tracking_status_stale_seconds', 2)),
                '--reconnect-min-ms', str(c['reconnect_min_ms']), '--reconnect-max-ms', str(c['reconnect_max_ms'])]

    def start(self):
        if self.worker is not None:
            raise RuntimeError('MAVLink client already started')
        self.stop_event.clear()
        self.process = subprocess.Popen(self.command_line(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, bufsize=0)
        os.set_blocking(self.process.stdin.fileno(), False)
        os.set_blocking(self.process.stdout.fileno(), False)
        self.worker = threading.Thread(target=self._io_loop, name='mavlink-ipc', daemon=True)
        self.worker.start()

    def _handle(self, event):
        kind = event['type']
        if kind == 'connection':
            self.connected = event['connected']
            self.selection.clear()
            if not self.connected:
                self.outbox.clear()
            LOG.info('TCP router %s connected=%s', event['endpoint'], self.connected)
        elif kind == 'track_point':
            self.selection.set_point(event['x'], event['y'], event['radius'], generation=event['generation'])
            LOG.info('Selection point x=%.3f y=%.3f radius=%.3f', event['x'], event['y'], event['radius'])
        elif kind == 'clear_selection':
            self.selection.clear(generation=event['generation'])
        elif kind == 'error':
            LOG.warning('Native bridge: %s', event['error'])
        with self.condition:
            self.events.append(event)
            self.condition.notify_all()

    def _io_loop(self):
        pending = b''
        received = b''
        try:
            while not self.stop_event.is_set():
                if self.process.poll() is not None:
                    LOG.error('Native bridge exited with status %s', self.process.returncode)
                    break
                if not pending:
                    event = self.outbox.pop()
                    if event is not None:
                        pending = (json.dumps(event, allow_nan=False) + '\n').encode()
                reads, writes, _ = select.select([self.process.stdout], [self.process.stdin] if pending else [], [], .02)
                if reads:
                    chunk = os.read(self.process.stdout.fileno(), 4096)
                    if not chunk:
                        break
                    received += chunk
                    while b'\n' in received:
                        line, received = received.split(b'\n', 1)
                        self._handle(json.loads(line))
                    if len(received) > 16384:
                        raise ValueError('native IPC event exceeds 16 KiB')
                if writes:
                    try:
                        pending = pending[os.write(self.process.stdin.fileno(), pending):]
                    except BlockingIOError:
                        pass
        except (OSError, ValueError, KeyError):
            if not self.stop_event.is_set():
                LOG.exception('MAVLink IPC worker failed')
        finally:
            self.connected = False
            self.selection.clear()
            self.outbox.clear()

    def request_get(self, request_id, key):
        if not self.connected:
            return False
        return self.outbox.put({'type': 'get', 'request_id': request_id, 'key': key})

    def telemetry(self, **fields):
        if not self.connected:
            return False
        return self.outbox.put({'type': 'telemetry', **fields})

    def select(self, detections, frame_size):
        selected, state = self.selection.evaluate(detections, frame_size)
        if self.connected:
            self.outbox.put({'type': 'selection_state', **state})
        return selected

    def next_event(self, predicate, timeout):
        deadline = time.monotonic() + timeout
        with self.condition:
            while True:
                for event in self.events:
                    if predicate(event):
                        self.events.remove(event)
                        return event
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    raise TimeoutError('MAVLink event timeout')
                self.condition.wait(remaining)

    def stop(self):
        self.stop_event.set()
        if self.worker:
            self.worker.join(timeout=1)
        if self.process:
            self.process.terminate()
            try:
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.process.kill(); self.process.wait()
            self.process.stdin.close(); self.process.stdout.close()
        self.selection.clear()
        self.worker = None
