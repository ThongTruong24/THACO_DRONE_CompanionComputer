"""QGC camera protocol reproduction through REAL router UDP14550 and TCP5760.

Not a QGC executable/UI test. Packet helper MUST use independently generated QGC
all headers. Router config is rendered by the unchanged production generator.
"""
import argparse
from collections import deque
import json
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from mavlink_client import MavlinkClient
from target_selection import Detection
from bbox_renderer import render_plan


def run(args):
    packets = args.build_dir / 'qgc-camera-packets'
    metadata = json.loads(subprocess.check_output([str(packets), 'metadata']))
    assert metadata['dialect'] == 'all' and metadata['revision']
    assert (metadata['information_crc'], metadata['status_crc']) == (92, 126)
    reservations = []
    processes = []
    client = None
    udp = None
    with tempfile.TemporaryDirectory(prefix='vision-camera-router-') as directory:
        root = Path(directory)
        router_log = root / 'router.log'
        log = router_log.open('w')
        try:
            # Refuse overlap rather than touching any existing stack.
            for kind, port in [(socket.SOCK_STREAM, 5760), (socket.SOCK_DGRAM, 14550),
                               (socket.SOCK_DGRAM, 14541), (socket.SOCK_DGRAM, 14600)]:
                s = socket.socket(socket.AF_INET, kind)
                if kind == socket.SOCK_STREAM:
                    # Permit a previous fixture's TIME_WAIT, while an active
                    # listener still makes this reservation fail.
                    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                s.bind(('0.0.0.0', port)); reservations.append(s)
            config = subprocess.check_output([str(args.build_dir / 'render-baseline-config')], text=True)
            assert 'TcpServerPort = 5760' in config and 'Port = 14550' in config
            (root / 'main.conf').write_text(config)
            (root / 'config.d').mkdir()
            for s in reservations: s.close()
            reservations.clear()
            router = subprocess.Popen([str(args.router_binary), '-c', str(root / 'main.conf'),
                                       '-d', str(root / 'config.d')], stdout=log, stderr=subprocess.STDOUT)
            processes.append(router)
            client = MavlinkClient(dict(router_host='127.0.0.1', router_port=5760, system_id=1,
                                       component_id=192, camera_component_id=args.alias,
                                       tracking_status_max_hz=2, tracking_status_stale_seconds=2,
                                       reconnect_min_ms=20, reconnect_max_ms=100), selection_ttl=10,
                                   executable=str(args.bridge_executable or args.build_dir / 'mavlink-bridge'))
            client.start()
            client.next_event(lambda e: e['type'] == 'connection' and e['connected'], 5)
            udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            udp.bind(('127.0.0.1', 0)); udp.connect(('127.0.0.1', 14550)); udp.settimeout(.2)
            udp.send(subprocess.check_output([str(packets), 'heartbeat']))
            mailbox = deque(maxlen=256)
            sequences = {}

            def receive(predicate, timeout=5):
                deadline = time.monotonic() + timeout
                while time.monotonic() < deadline:
                    for row in list(mailbox):
                        if predicate(row): mailbox.remove(row); return row
                    assert router.poll() is None, 'router exited'
                    try: data = udp.recv(65535)
                    except socket.timeout: continue
                    rows = json.loads(subprocess.check_output([str(packets), 'decode'], input=data))
                    for row in rows:
                        assert row['id'] != 269, 'VIDEO_STREAM_INFORMATION must not override manual RTSP'
                        if row['sysid'] == 1 and row['compid'] in (192, args.alias):
                            previous = sequences.get(row['compid'])
                            if previous is not None:
                                assert (row['sequence'] - previous) % 256 > 0, 'logical component reused packet sequence'
                            sequences[row['compid']] = row['sequence']
                        row['received_at'] = time.monotonic(); mailbox.append(row)
                raise TimeoutError('QGC-compatible UDP message timeout')

            def send(command, *params, component=None):
                udp.send(subprocess.check_output([str(packets), 'command', '1',
                                                   str(args.alias if component is None else component),
                                                   str(command), *map(str, params)]))

            def ack(command, result=0):
                row = receive(lambda r: r['id'] == 77 and r['command'] == command and r['result'] == result)
                assert (row['sysid'], row['compid']) == (1, args.alias)
                assert (row['target_system'], row['target_component']) == (255, 190)
                return row

            heartbeats = [receive(lambda r: r['id'] == 0 and r['sysid'] == 1 and r['compid'] == component)
                          for component in (192, args.alias)]
            assert [r['type'] for r in heartbeats] == [18, 30]
            assert all(r['autopilot'] == 8 for r in heartbeats)
            client.telemetry(confidence=.35, inference_fps=5, width=640, height=360, fps=20, count=1, model='fixture')
            telemetry = receive(lambda r: r['id'] == 42013)
            assert (telemetry['sysid'], telemetry['compid']) == (1, 192)
            assert (telemetry['width'], telemetry['height'], telemetry['count']) == (640, 360, 1)
            # Production dispatch is on one native process/TCP peer, not two helpers.
            for command, parameter in [(512, 259), (521, 1)]:
                send(command, parameter); ack(command)
                info = receive(lambda r: r['id'] == 259 and r['compid'] == args.alias)
                assert info['flags'] == 512 and info['vendor'] == 'THACO'
                assert info['model'] == 'AgriDrone Vision Selector'

            send(2004, .5, .5, .1, component=192)
            try: client.next_event(lambda e: e['type'] == 'track_point', .15)
            except TimeoutError: pass
            else: raise AssertionError('primary192 must not dispatch camera alias tracking')

            send(2004, .5, .5, .1); ack(2004)
            client.next_event(lambda e: e['type'] == 'track_point' and e['target_compid'] == args.alias, 3)
            item = Detection((40, 40, 60, 60), .9, 'fixture-target')
            assert client.select([item], (100, 100)) == item
            assert render_plan([item], item)[0].color == (0, 255, 255)
            send(511, 275, 500000); ack(511)
            active = receive(lambda r: r['id'] == 275 and r['tracking_status'] == 1)
            assert active['compid'] == args.alias and active['tracking_mode'] == 1
            assert active['target_data'] == 6 and active['x'] == .5 and active['y'] == .5
            assert 0 < active['radius'] < 1
            times = [active['received_at']]
            for _ in range(2):
                client.select([item], (100, 100))
                times.append(receive(lambda r: r['id'] == 275 and r['tracking_status'] == 1)['received_at'])
            assert all(b-a >= .45 for a, b in zip(times, times[1:])), times
            client.select([], (100, 100))
            lost = receive(lambda r: r['id'] == 275 and r['tracking_status'] == 0)
            assert lost['tracking_mode'] == 0 and lost['x'] is None
            client.select([item], (100, 100))
            receive(lambda r: r['id'] == 275 and r['tracking_status'] == 1)
            # Width-normalized radius must reject a box that only the old
            # diagonal interpretation would include on a tall frame.
            send(2004, .5, .5, .1); ack(2004)
            client.next_event(lambda e: e['type'] == 'track_point', 3)
            assert client.select([Detection((62, 500, 70, 510), .9, 'outside')], (100, 1000)) is None
            receive(lambda r: r['id'] == 275 and r['tracking_status'] == 0)
            send(2004, .5, .5, .1); ack(2004)
            client.next_event(lambda e: e['type'] == 'track_point', 3)
            assert client.select([item], (100, 100)) == item
            receive(lambda r: r['id'] == 275 and r['tracking_status'] == 1)
            send(512, 259); ack(512)
            info = receive(lambda r: r['id'] == 259 and r['compid'] == args.alias)
            assert (info['width'], info['height']) == (100, 100)
            send(2010); ack(2010)
            client.next_event(lambda e: e['type'] == 'clear_selection', 3)
            assert client.select([item], (100, 100)) is None
            assert render_plan([item], None)[0].color == (0, 255, 0)
            idle = receive(lambda r: r['id'] == 275 and r['tracking_status'] == 0)
            assert idle['tracking_mode'] == 0
            # A short link outage can leave QGC's camera object alive. The
            # same helper must clear selection and resume requested IDLE feedback.
            send(2004, .5, .5, .1); ack(2004)
            client.next_event(lambda e: e['type'] == 'track_point', 3)
            client.select([item], (100, 100))
            receive(lambda r: r['id'] == 275 and r['tracking_status'] == 1)
            outage_started = time.time()
            router.terminate(); router.wait(timeout=5)
            # Ignore any retained startup-disconnected event in the mailbox.
            client.next_event(lambda e: e['type'] == 'connection' and not e['connected']
                              and e['timestamp'] >= outage_started, 3)
            assert client.selection.point is None
            mailbox.clear()
            router = subprocess.Popen([str(args.router_binary), '-c', str(root / 'main.conf'),
                                       '-d', str(root / 'config.d')], stdout=log, stderr=subprocess.STDOUT)
            processes.append(router)
            client.next_event(lambda e: e['type'] == 'connection' and e['connected'], 5)
            udp.send(subprocess.check_output([str(packets), 'heartbeat']))
            resumed = receive(lambda r: r['id'] == 275)
            assert resumed['tracking_status'] == 0 and resumed['tracking_mode'] == 0
            send(511, 275, -1); ack(511)
            mailbox.clear()
            try: receive(lambda r: r['id'] == 275, .7)
            except TimeoutError: pass
            else: raise AssertionError('status reporting did not disable')
            send(512, 269); ack(512, result=3)
            print(json.dumps({'result': 'PASS', 'fixture': 'QGC camera protocol, not actual QGC UI',
                              'dialect': metadata, 'router_udp': 14550, 'vision_tcp': 5760,
                              'identities': [f'1/192', f'1/{args.alias}'], 'status_intervals_s':
                              [round(b-a, 3) for a, b in zip(times, times[1:])],
                              'checks': ['both heartbeats and sequence counters', 'primary42013', '512/521 discovery', 'capabilities', 'alias-only point',
                                         'ACK source/target', 'width radius', '2Hz status', 'loss idle', 'STOP idle',
                                         'reconnect idle', 'disable interval', 'no automatic video stream']}), flush=True)
        except Exception:
            log.flush(); print(router_log.read_text(), file=sys.stderr); raise
        finally:
            if client: client.stop()
            if udp: udp.close()
            for s in reservations: s.close()
            for process in reversed(processes):
                process.terminate()
                try: process.wait(timeout=5)
                except subprocess.TimeoutExpired: process.kill(); process.wait()
            log.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--router-binary', type=Path, required=True)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--bridge-executable', type=Path)
    parser.add_argument('--alias', type=int, default=105)
    run(parser.parse_args())
