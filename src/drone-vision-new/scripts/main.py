import argparse
import logging
import os
import signal
import subprocess
import threading
from configuration import load_config
from mavlink_client import MavlinkClient
from yolo_streamer import LatestFrame, capture_loop, inference_loop, publish_loop

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true', help='Validate packaged configuration, model and native codec without networking')
    args = parser.parse_args()
    logging.basicConfig(level=os.getenv('LOG_LEVEL', 'INFO'), format='%(asctime)s %(levelname)s %(name)s %(message)s')
    config = load_config(os.getenv('VISION_CONFIG', '/app/config/drone.yaml'))
    client = MavlinkClient(config['mavlink'], config['selection']['ttl_seconds'])
    if args.check:
        subprocess.run(client.command_line() + ['--self-test'], check=True)
        print('Packaged configuration and YOLO model are present.')
        return
    c = config['yolo']
    os.environ['OMP_NUM_THREADS'] = str(c['threads'])
    import gi
    gi.require_version('Gst', '1.0')
    from gi.repository import Gst
    import torch
    from ultralytics import YOLO
    Gst.init(None)
    torch.set_num_threads(c['threads'])
    torch.set_num_interop_threads(1)
    model = YOLO(c['model'], task='detect')
    stop = threading.Event()
    for sig in (signal.SIGINT, signal.SIGTERM):
        signal.signal(sig, lambda *_: stop.set())
    frames, annotated = LatestFrame(), LatestFrame()
    workers = [threading.Thread(target=capture_loop, args=(c, frames, stop), name='capture', daemon=True),
               threading.Thread(target=inference_loop, args=(c, model, frames, annotated, client, stop), name='inference', daemon=True),
               threading.Thread(target=publish_loop, args=(c, Gst, annotated, client, stop), name='publish', daemon=True)]
    try:
        client.start()
        for worker in workers:
            worker.start()
        logging.info('drone-vision-new ready: %s -> %s; MAVLink TCP %s:%s (%s/%s)',
                     c['input_url'], c['output_url'], config['mavlink']['router_host'],
                     config['mavlink']['router_port'], config['mavlink']['system_id'], config['mavlink']['component_id'])
        logging.info('Native QGC camera alias: %s/%s; manual /yolo RTSP; status cap %s Hz',
                     config['mavlink']['system_id'], config['mavlink']['camera_component_id'],
                     config['mavlink']['tracking_status_max_hz'])
        while not stop.wait(.5):
            if any(not worker.is_alive() for worker in workers):
                raise RuntimeError('Vision worker exited unexpectedly')
    finally:
        stop.set()
        client.stop()
        for worker in workers:
            if worker.ident is not None:
                worker.join(timeout=6)

if __name__ == '__main__':
    main()
