"""Production mounts must work without host-side permission/directory repairs."""
import os
import shutil
import subprocess
import uuid
from pathlib import Path

import pytest
import yaml

ROOT = Path(__file__).resolve().parents[4]


@pytest.mark.parametrize("service,config_name", [
    ("edge-vision-v1", "vision_v1.yaml"), ("edge-camera", "camera_streamer.yaml"),
])
def test_config_mounts_are_siblings_and_entrypoint_is_image_owned(service, config_name):
    config = yaml.safe_load((ROOT / "docker-compose.yml").read_text())
    volumes = config["services"][service]["volumes"]
    mounts = [item for item in volumes if isinstance(item, dict)]
    assert {item["target"] for item in mounts} == {
        f"/app/config/{config_name}", "/app/config/cameras",
    }
    for item in mounts:
        assert item["read_only"]
        assert not item["bind"]["create_host_path"]
        assert (ROOT / item["source"]).exists()
    assert not any(isinstance(item, str) and ":/app/entrypoint.sh:" in item for item in volumes)
    module = "vision_v1" if service == "edge-vision-v1" else "camera_streamer"
    assert not (ROOT / f"src/modules/{module}/config/cameras").exists()


def test_vision_entrypoint_can_be_read_by_bash_without_host_exec_permission(tmp_path):
    # Stub ros2: exercise actual argument assembly without starting workers or ROS.
    original = ROOT / "src/modules/vision_v1/entrypoint.sh"
    script = tmp_path / "entrypoint.sh"
    script.write_text(original.read_text())
    script.chmod(0o644)
    ros2 = tmp_path / "ros2"
    ros2.write_text('#!/bin/bash\nprintf "%s\\n" "$@"\n')
    ros2.chmod(0o755)
    result = subprocess.run(
        ["bash", str(script), "--ros-args", "--params-file", "/app/config/vision_v1.yaml"],
        env={**os.environ, "PATH": str(tmp_path) + ":" + os.environ["PATH"], "CAMERA_MODEL": "not_present"},
        capture_output=True, text=True, check=True,
    )
    assert result.stdout.splitlines() == [
        "run", "vision_v1", "vision_v1_node", "--ros-args", "--params-file", "/app/config/vision_v1.yaml",
    ]


def test_camera_profile_arguments_are_inside_ros_group_and_precede_module_config(tmp_path):
    original = ROOT / "src/modules/vision_v1/entrypoint.sh"
    config = tmp_path / "config"
    (config / "cameras").mkdir(parents=True)
    profile = config / "cameras/realsense_d435i.yaml"
    profile.write_text("/**:\n  ros__parameters:\n    fps: 30\n")
    script = tmp_path / "entrypoint.sh"
    script.write_text(original.read_text().replace("/app/config", str(config)))
    ros2 = tmp_path / "ros2"
    ros2.write_text('#!/bin/bash\nprintf "%s\\n" "$@"\n')
    ros2.chmod(0o755)
    result = subprocess.run(
        ["bash", str(script), "--ros-args", "--params-file", str(config / "vision_v1.yaml")],
        env={**os.environ, "PATH": str(tmp_path) + ":" + os.environ["PATH"], "CAMERA_MODEL": "realsense_d435i"},
        capture_output=True, text=True, check=True,
    )
    arguments = result.stdout.splitlines()[1:]
    assert arguments == ["run", "vision_v1", "vision_v1_node", "--ros-args", "--params-file",
                         str(profile), "--", "--ros-args", "--params-file", str(config / "vision_v1.yaml")]


@pytest.mark.skipif(os.environ.get("VISION_DOCKER_TEST") != "1", reason="set VISION_DOCKER_TEST=1 for Docker smoke")
def test_compose_clean_staging_mounts_image_entrypoint(tmp_path):
    shutil.copy(ROOT / "docker-compose.yml", tmp_path)
    for relative in ["src/modules/vision_v1/config/vision_v1.yaml",
                     "src/modules/camera_streamer/config/camera_streamer.yaml",
                     "src/modules/vision_v1/entrypoint.sh"]:
        target = tmp_path / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy(ROOT / relative, target)
    (tmp_path / "src/modules/vision_v1/entrypoint.sh").chmod(0o644)
    profiles = tmp_path / "src/drivers/camera/config"
    shutil.copytree(ROOT / "src/drivers/camera/config", profiles)
    model = tmp_path / "src/modules/vision/yolo26n.pt"
    model.parent.mkdir(parents=True)
    model.symlink_to(ROOT / "src/modules/vision/yolo26n.pt")
    command = ["docker", "compose", "--project-name", "vision-smoke-" + uuid.uuid4().hex[:8],
               "--profile", "vision-v1", "-f", str(tmp_path / "docker-compose.yml")]
    try:
        result = subprocess.run(command + ["run", "--rm", "--no-deps", "--entrypoint", "/bin/bash",
                                           "edge-vision-v1", "-c",
                                           ("test -x /app/entrypoint.sh && test -f /app/config/vision_v1.yaml "
                                            "&& test -f /app/config/cameras/realsense_d435i.yaml")],
                                capture_output=True, text=True, timeout=90, check=False)
        assert result.returncode == 0, result.stdout + result.stderr
        assert not (tmp_path / "src/modules/vision_v1/config/cameras").exists()
    finally:
        subprocess.run(command + ["down", "--volumes"], capture_output=True, timeout=30, check=False)
