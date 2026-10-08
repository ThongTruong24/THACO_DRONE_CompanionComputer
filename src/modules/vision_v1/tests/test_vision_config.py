from pathlib import Path

import yaml
from vision_v1.publisher import (
    DEFAULT_BITRATE_KBPS, DEFAULT_ENCODER_PRESET, DEFAULT_KEY_INT_MAX, make_pipeline_desc,
)


def test_production_config_contains_decoupled_sources_and_rates():
    path = Path(__file__).resolve().parents[1] / "config" / "vision_v1.yaml"
    parameters = yaml.safe_load(path.read_text())["/**"]["ros__parameters"]
    assert parameters["source"] == "/camera/frame_ready"
    assert parameters["input_url"].endswith("/camera")
    assert parameters["output_url"].endswith("/yolo")
    assert parameters["detection_max_age"] > 0
    assert parameters["reconnect_seconds"] > 0
    assert parameters["fps"] != parameters["inference_fps"]
    assert parameters["fps"] == 30
    assert parameters["bitrate_kbps"] == DEFAULT_BITRATE_KBPS == 2000
    assert parameters["encoder_preset"] == DEFAULT_ENCODER_PRESET == "ultrafast"
    assert parameters["key_int_max"] == DEFAULT_KEY_INT_MAX == 30
    pipeline = make_pipeline_desc(
        parameters["width"], parameters["height"], parameters["fps"],
        parameters["bitrate_kbps"], parameters["output_url"],
        parameters["encoder_preset"], parameters["key_int_max"],
    )
    assert "bitrate=2000" in pipeline
