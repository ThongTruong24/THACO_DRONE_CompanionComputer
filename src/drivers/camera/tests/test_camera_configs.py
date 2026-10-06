import os
import yaml
import pytest

CONFIG_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../config"))

def test_realsense_d435i_config_structure():
    path = os.path.join(CONFIG_DIR, "realsense_d435i.yaml")
    assert os.path.exists(path), f"{path} does not exist"
    with open(path, "r") as f:
        data = yaml.safe_load(f)
    params = data["/**"]["ros__parameters"]
    assert params["camera_model"] == "realsense_d435i"
    assert params["camera_type"] == "realsense"
    assert params["video"]["width"] == 1280
    assert params["video"]["height"] == 720
    assert params["video"]["rotation"] == 180
    assert params["fps"] == 30
    assert "realsense" in params
    assert params["realsense"]["depth_width"] == 640
    assert params["realsense"]["depth_height"] == 480
    assert params["min_distance_m"] == 0.3
    assert params["max_distance_m"] == 10.0

def test_v4l2_default_config_structure():
    path = os.path.join(CONFIG_DIR, "v4l2_default.yaml")
    assert os.path.exists(path), f"{path} does not exist"
    with open(path, "r") as f:
        data = yaml.safe_load(f)
    params = data["/**"]["ros__parameters"]
    assert params["camera_model"] == "v4l2_default"
    assert params["camera_type"] == "v4l2"
    assert "v4l2" in params
    assert params["v4l2"]["device_path"] == "/dev/video0"
