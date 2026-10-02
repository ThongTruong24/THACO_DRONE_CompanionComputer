import copy
import ipaddress
import math
import os
from pathlib import Path
import yaml

def validate_config(raw, check_model=True):
    if not isinstance(raw, dict):
        raise ValueError('configuration must be a mapping')
    c = copy.deepcopy(raw)
    m, y, selection = c['mavlink'], c['yolo'], c['selection']
    if ipaddress.ip_address(m['router_host']).version != 4:
        raise ValueError('mavlink.router_host must be numeric IPv4')
    for key, maximum in [('router_port', 65535), ('system_id', 255), ('component_id', 255)]:
        if type(m[key]) is not int or not 1 <= m[key] <= maximum:
            raise ValueError(f'mavlink.{key} is outside 1..{maximum}')
    if m['component_id'] == 191:
        raise ValueError('component 191 is reserved for the existing companion agent')
    if type(m['camera_component_id']) is not int or not 100 <= m['camera_component_id'] <= 105:
        raise ValueError('mavlink.camera_component_id must be an integer in 100..105')
    if m['camera_component_id'] == m['component_id']:
        raise ValueError('camera alias must differ from the primary Vision component')
    for key in ('tracking_status_max_hz', 'tracking_status_stale_seconds'):
        value = m[key]
        if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value <= 0:
            raise ValueError(f'mavlink.{key} must be finite and positive')
    if m['tracking_status_max_hz'] > 10:
        raise ValueError('mavlink.tracking_status_max_hz must not exceed 10 Hz')
    for key in ('reconnect_min_ms', 'reconnect_max_ms'):
        if type(m[key]) is not int or not 1 <= m[key] <= 60000:
            raise ValueError(f'mavlink.{key} must be an integer in 1..60000')
    if m['reconnect_max_ms'] < m['reconnect_min_ms']:
        raise ValueError('reconnect maximum must be >= minimum')
    for key in ('width', 'height', 'fps', 'imgsz', 'threads', 'bitrate_kbps'):
        if type(y[key]) is not int or y[key] <= 0:
            raise ValueError(f'yolo.{key} must be a positive integer')
    if y['width'] % 2 or y['height'] % 2 or max(y['width'], y['height']) > 65535 or y['fps'] > 255:
        raise ValueError('H.264 dimensions must be even and fit telemetry fields; fps must be <=255')
    for value in (y['inference_fps'], y['stale_seconds'], y['reconnect_seconds'], selection['ttl_seconds']):
        if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value <= 0:
            raise ValueError('rates and timeouts must be finite and positive')
    if isinstance(y['confidence'], bool) or not isinstance(y['confidence'], (int, float)) or not 0 < y['confidence'] <= 1:
        raise ValueError('yolo.confidence must be in (0, 1]')
    if not y['input_url'].startswith('rtsp://') or not y['output_url'].startswith('rtmp://127.0.0.1:'):
        raise ValueError('input must be RTSP; output must use the existing local RTMP server')
    if check_model and not Path(y['model']).is_file():
        raise FileNotFoundError(y['model'])
    return c

def load_config(path, check_model=True):
    with open(path, encoding='utf-8') as handle:
        config = yaml.safe_load(handle)
    for env, key, converter in [('MAVLINK_ROUTER_HOST', 'router_host', str),
                                ('MAVLINK_ROUTER_PORT', 'router_port', int),
                                ('MAVLINK_SYSTEM_ID', 'system_id', int),
                                ('MAVLINK_COMPONENT_ID', 'component_id', int),
                                ('MAVLINK_CAMERA_COMPONENT_ID', 'camera_component_id', int),
                                ('MAVLINK_TRACKING_STATUS_MAX_HZ', 'tracking_status_max_hz', float)]:
        if env in os.environ:
            config['mavlink'][key] = converter(os.environ[env])
    return validate_config(config, check_model)
