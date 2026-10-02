"""TCP test-server integration for the packaged IPC/command receive path."""
from pathlib import Path
import socket
import subprocess
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from mavlink_client import MavlinkClient

BUILD = Path(sys.argv[1])

class IpcTcpTests(unittest.TestCase):
    def test_target_filtering_tracking_ack_reconnect_and_timeout(self):
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind(('127.0.0.1', 0)); server.listen(); server.settimeout(5)
        client = MavlinkClient(dict(router_host='127.0.0.1', router_port=server.getsockname()[1], system_id=1,
                                  component_id=192, reconnect_min_ms=20, reconnect_max_ms=100),
                               executable=str(BUILD / 'mavlink-bridge'))
        peer = None
        try:
            client.start(); peer, _ = server.accept(); peer.settimeout(2)
            client.next_event(lambda e: e['type'] == 'connection' and not e['connected'], 2)
            client.next_event(lambda e: e['type'] == 'connection' and e['connected'], 2)
            def command(sysid, compid, x=.25):
                return subprocess.check_output([str(BUILD / 'packet-fixture'), 'track', str(sysid), str(compid), str(x)])
            peer.sendall(command(2, 105)); peer.sendall(command(1, 104)); peer.sendall(command(1, 192))
            with self.assertRaises(TimeoutError): client.next_event(lambda e: e['type'] == 'track_point', .1)
            packet = command(1, 105)
            peer.sendall(packet[:5])
            with self.assertRaises(TimeoutError): client.next_event(lambda e: e['type'] == 'track_point', .05)
            peer.sendall(packet[5:])
            event = client.next_event(lambda e: e['type'] == 'track_point', 2)
            self.assertEqual((event['sysid'], event['compid']), (255, 190))
            self.assertEqual((event['target_sysid'], event['target_compid']), (1, 105))
            self.assertEqual((event['x'], event['y']), (.25, .75))
            import json
            import time
            def wait_ack(result):
                data = b''; deadline = time.monotonic() + 3
                while time.monotonic() < deadline:
                    data += peer.recv(4096)
                    decoded = json.loads(subprocess.check_output([str(BUILD / 'packet-fixture'), 'decode'], input=data))
                    for message in decoded:
                        if message['id'] == 77 and message['result'] == result:
                            return message
                raise TimeoutError('COMMAND_ACK not received')
            ack = wait_ack(0)
            self.assertEqual((ack['sysid'], ack['compid']), (1, 105))
            self.assertEqual((ack['target_sysid'], ack['target_compid']), (255, 190))
            self.assertEqual(ack['command'], 2004)
            peer.sendall(command(1, 105, 1.2))
            self.assertEqual(wait_ack(2)['command'], 2004)
            with self.assertRaises(TimeoutError): client.next_event(lambda e: e['type'] == 'track_point', .05)
            self.assertIsNotNone(client.selection.point)
            peer.close(); peer = None
            client.next_event(lambda e: e['type'] == 'connection' and not e['connected'], 2)
            self.assertIsNone(client.selection.point)
            peer, _ = server.accept()
            client.next_event(lambda e: e['type'] == 'connection' and e['connected'], 2)
        finally:
            client.stop()
            if peer: peer.close()
            server.close()

if __name__ == '__main__': unittest.main(argv=[sys.argv[0]], verbosity=2)
