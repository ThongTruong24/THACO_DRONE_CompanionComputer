import os
import yaml
import pytest

CONFIG_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "../config/camera_streamer.yaml"))

def test_camera_streamer_decoupled_config():
    assert os.path.exists(CONFIG_PATH), f"{CONFIG_PATH} does not exist"
    with open(CONFIG_PATH, "r") as f:
        data = yaml.safe_load(f)
    params = data["/**"]["ros__parameters"]
    assert params["camera_model"] == "realsense_d435i"
    assert "bitrate_kbps" in params
    assert "encoder" in params
    assert "network" in params
    # Hardware sensor params should not be hardcoded here
    assert "realsense" not in params
    assert "v4l2" not in params
