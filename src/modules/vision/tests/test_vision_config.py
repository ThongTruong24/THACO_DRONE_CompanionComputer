import os
import yaml
import pytest

CONFIG_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "../config/vision.yaml"))

def test_vision_config_has_camera_model():
    assert os.path.exists(CONFIG_PATH), f"{CONFIG_PATH} does not exist"
    with open(CONFIG_PATH, "r") as f:
        data = yaml.safe_load(f)
    params = data["/**"]["ros__parameters"]
    assert params.get("camera_model") == "realsense_d435i"
    assert "model" in params
    assert "confidence" in params
