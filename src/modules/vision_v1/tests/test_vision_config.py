from pathlib import Path

import yaml


def test_production_config_contains_decoupled_sources_and_rates():
    path = Path(__file__).resolve().parents[1] / "config" / "vision_v1.yaml"
    parameters = yaml.safe_load(path.read_text())["/**"]["ros__parameters"]
    assert parameters["source"] == "/camera/frame_ready"
    assert parameters["input_url"].endswith("/camera")
    assert parameters["output_url"].endswith("/yolo")
    assert parameters["detection_max_age"] > 0
    assert parameters["reconnect_seconds"] > 0
    assert parameters["fps"] != parameters["inference_fps"]
