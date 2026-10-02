"""Local-only real daemon tests. Never contact a Pi or start production containers."""
import argparse
import os
from pathlib import Path
import socket
import subprocess
import tempfile
import time
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from mavlink_client import MavlinkClient

ROOT = Path(__file__).resolve().parents[3]

def reserve(kind, port=0):
    sock = socket.socket(socket.AF_INET, kind)
    sock.bind(('127.0.0.1', port))
    return sock

def run(args):
    with tempfile.TemporaryDirectory(prefix='vision-new-integration-') as directory:
        root = Path(directory)
        for name in ('config.d', 'agent-root', 'agent-run', 'agent-state'):
            (root / name).mkdir()
        config = subprocess.check_output([str(args.build_dir / 'render-baseline-config')], text=True)
        assert 'TcpServerPort = 5760' in config
        reservations = [reserve(socket.SOCK_STREAM, 5760 if args.production_ports else 0)]
        reservations += [reserve(socket.SOCK_DGRAM, p if args.production_ports else 0) for p in (14550, 14541, 14600)]
        tcp, qgc, peer, agent_port = [s.getsockname()[1] for s in reservations]
        if args.mode == 'cc-agent':
            # Unchanged agent always binds 14601. Refuse to overlap an existing process.
            reservations.append(reserve(socket.SOCK_DGRAM, 14601))
        config = config.replace('TcpServerPort = 5760', f'TcpServerPort = {tcp}')
        for original, replacement in ((14550, qgc), (14541, peer), (14600, agent_port)):
            config = config.replace(f'Port = {original}\n', f'Port = {replacement}\n')
        (root / 'main.conf').write_text(config)
        processes = []
        client = None
        logs = []
        try:
            for reservation in reservations:
                reservation.close()
            log = open(root / 'router.log', 'w'); logs.append(log)
            router = subprocess.Popen([str(args.router_binary), '-c', str(root / 'main.conf'), '-d', str(root / 'config.d')], stdout=log, stderr=subprocess.STDOUT)
            processes.append(router)
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline:
                if router.poll() is not None:
                    raise RuntimeError('real router exited before readiness')
                try:
                    with socket.create_connection(('127.0.0.1', tcp), timeout=.1): break
                except OSError: time.sleep(.05)
            else: raise TimeoutError('router TCP server readiness timeout')
            if args.mode == 'cc-agent':
                if not args.agent_binary or not args.agent_binary.is_file():
                    raise FileNotFoundError('provide an unchanged baseline native cc-agent binary')
                env = dict(os.environ, DRONE_ROOT=str(root / 'agent-root'), DRONE_RUN_DIR=str(root / 'agent-run'), DRONE_CONFIG_DIR=str(root / 'agent-state'))
                log = open(root / 'agent.log', 'w'); logs.append(log)
                processes.append(subprocess.Popen([str(args.agent_binary), '127.0.0.1', str(agent_port)], env=env, stdout=log, stderr=subprocess.STDOUT))
            subprocess.run([str(args.build_dir / 'router-probe'), str(tcp), str(peer), args.mode], check=True, timeout=15)
            if args.mode == 'cc-agent':
                # Also exercise the final Python -> pipe -> native bridge path, not only its transport library.
                client = MavlinkClient(dict(router_host='127.0.0.1', router_port=tcp, system_id=1, component_id=192,
                                           reconnect_min_ms=100, reconnect_max_ms=5000), executable=str(args.bridge_executable or args.build_dir / 'mavlink-bridge'))
                client.start()
                client.next_event(lambda e: e['type'] == 'connection' and e['connected'], 5)
                client.next_event(lambda e: e['type'] == 'heartbeat' and e['sysid'] == 1 and e['compid'] == 191, 5)
                started = time.monotonic()
                assert client.request_get(0x5a170001, 'telemetry.fc.baudrate')
                tx = client.next_event(lambda e: e['type'] == 'tx' and e['queued'], 5)
                rx = client.next_event(lambda e: e['type'] == 'config_value' and e['request_id'] == 0x5a170001, 5)
                assert rx['message_id'] == 42107 and rx['sysid'] == 1 and rx['compid'] == 191
                assert rx['key'] == 'telemetry.fc.baudrate' and rx['value'] == '921600'
                print(f'PASS packaged IPC roundtrip: TX={tx} RX={rx} latency_ms={(time.monotonic()-started)*1000:.3f}')
                assert not list((root / 'agent-state').iterdir()), 'GET must not persist configuration'
                assert not list((root / 'agent-root').iterdir()), 'GET must not write telemetry.env'
        except Exception:
            for log in logs: log.flush()
            for path in root.glob('*.log'): print(path.read_text(), file=sys.stderr)
            raise
        finally:
            if client: client.stop()
            for process in reversed(processes):
                process.terminate()
                try: process.wait(timeout=5)
                except subprocess.TimeoutExpired: process.kill(); process.wait()
            for log in logs: log.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--router-binary', type=Path, required=True)
    parser.add_argument('--agent-binary', type=Path)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--bridge-executable', type=Path, help='Optional executable wrapper for the final ARM64 image bridge')
    parser.add_argument('--mode', choices=['router', 'cc-agent'], required=True)
    parser.add_argument('--production-ports', action='store_true', help='Only on an isolated development host: assert exact baseline ports are free')
    run(parser.parse_args())
