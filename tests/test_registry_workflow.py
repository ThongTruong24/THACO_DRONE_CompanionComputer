"""Deployment contract tests using fake Git/Docker; no build, push or Pi access."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
REVISION = "a" * 40
spec = importlib.util.spec_from_file_location("validate_image", ROOT / "deploy/build/validate_image.py")
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)

MOCK = r'''#!/usr/bin/env python3
import json,os,pathlib,sys
name=pathlib.Path(sys.argv[0]).name; args=sys.argv[1:]
with open(os.environ["MOCK_LOG"],"a") as f: f.write(json.dumps([name,*args])+"\n")
revision=os.environ.get("MOCK_IMAGE_REVISION","a"*40)
image={"os":"linux","architecture":os.environ.get("MOCK_IMAGE_ARCH","arm64"),"config":{"Labels":{"org.opencontainers.image.revision":revision}}}
if name=="uname": print(os.environ.get("MOCK_HOST_ARCH","x86_64")); sys.exit()
if name in ("ss","ip"):
 print("192.168.10.1/24 :14550 :8554"); sys.exit()
if name=="git":
 if args[:1]==["-C"]: args=args[2:]
 if args[:1]==["status"]: print(os.environ.get("MOCK_DIRTY",""))
 elif args[:1]==["branch"]: print(os.environ.get("MOCK_BRANCH","dev/ai_tracking_flow"))
 elif args[:1]==["rev-parse"]: print("a"*40)
 elif args[:1]==["pull"]: sys.exit(int(os.environ.get("MOCK_GIT_PULL_FAILURE","0")))
 sys.exit()
if name=="docker":
 if args[:2]==["image","inspect"]: print(json.dumps([image])); sys.exit()
 if args[:1]==["inspect"]:
  print("sha256:fake" if "{{.Image}}" in args else "running none"); sys.exit()
 if args[:1]==["exec"]:
  if os.environ.get("MOCK_SOCKET_DELAY") and "/run/drone/hw_manager.sock" in args:
   marker=pathlib.Path(os.environ["MOCK_IMAGES"]+".socket")
   if not marker.exists(): marker.touch(); sys.exit(1)
  sys.exit(1 if "[UartEndpoint FlightController]" in args or any(v.startswith("Address = (192") for v in args) else 0)
 if args[:3]==["buildx","imagetools","inspect"]:
  state=pathlib.Path(os.environ["MOCK_IMAGES"])
  refs=json.loads(state.read_text()) if state.exists() else []
  if args[-1] not in refs: sys.exit(1)
  if "--raw" in args: print(json.dumps({"manifests":[{"platform":{"os":"linux","architecture":image["architecture"]}}]}))
  else: print(json.dumps(image if any("index .Image" in a for a in args) else {"linux/arm64":image}))
  sys.exit()
 if args[:2]==["buildx","build"]:
  state=pathlib.Path(os.environ["MOCK_IMAGES"])
  refs=json.loads(state.read_text()) if state.exists() else []
  refs.extend(args[i+1] for i,v in enumerate(args) if v=="--tag")
  state.write_text(json.dumps(refs)); sys.exit()
 if args[:1]==["compose"]:
  if "config" in args and "--format" in args: print(pathlib.Path(os.environ["MOCK_COMPOSE"]).read_text())
  elif "ps" in args and "-q" in args: print("container-id")
  sys.exit()
 sys.exit()
'''


class ImageValidationTest(unittest.TestCase):
    def test_attestation_does_not_hide_arm64(self):
        validator.validate_manifest({"manifests": [
            {"platform": {"os": "linux", "architecture": "arm64"}},
            {"platform": {"os": "unknown", "architecture": "unknown"},
             "annotations": {"vnd.docker.reference.type": "attestation-manifest"}},
        ]})

    def test_reject_amd64_only_and_mixed_platforms(self):
        for platforms in [["amd64"], ["arm64", "amd64"]]:
            with self.subTest(platforms=platforms), self.assertRaises(SystemExit):
                validator.validate_manifest({"manifests": [
                    {"platform": {"os": "linux", "architecture": a}} for a in platforms]})

    def test_reject_revision_mismatch(self):
        with self.assertRaises(SystemExit):
            validator.validate_config([{"Os": "linux", "Architecture": "arm64", "Config": {"Labels": {}}}], REVISION)


class RegistryWorkflowTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name)
        for command in ["docker", "git", "uname", "ss", "ip"]:
            p = self.path / command
            p.write_text(MOCK)
            p.chmod(0o755)
        self.env = os.environ.copy()
        self.env.update(PATH=f"{self.path}:{self.env['PATH']}", IMAGE_NAMESPACE="test-owner", IMAGE_TAG="dev",
                        REGISTRY="ghcr.io", EXPECTED_BRANCH="dev/ai_tracking_flow", BUILDER="drone-builder",
                        MOCK_LOG=str(self.path / "calls"), MOCK_IMAGES=str(self.path / "images"),
                        MOCK_COMPOSE=str(self.path / "compose.json"))
        services = {}
        for name in ["mavlink-router-controller", "cc-agent", "drone-networking", "camera-stream-controller", "mavros", "drone-vision"]:
            image = "drone-mavros" if name == "mavros" else name
            services[name] = {"image": f"ghcr.io/test-owner/{image}:dev", "volumes": []}
        services["mavros"]["depends_on"] = {"mavlink-router-controller": {}}
        services["drone-vision"]["depends_on"] = {"mavlink-router-controller": {}, "camera-stream-controller": {}}
        (self.path / "compose.json").write_text(json.dumps({"services": services}))

    def run_script(self, script, *args, **env):
        return subprocess.run(["bash", str(ROOT / script), *args], cwd=ROOT, env={**self.env, **env},
                              capture_output=True, text=True)

    def calls(self):
        p = self.path / "calls"
        return [json.loads(l) for l in p.read_text().splitlines()] if p.exists() else []

    def test_publish_requires_namespace_before_docker(self):
        r = self.run_script("deploy/build/images.sh", "publish", "core", IMAGE_NAMESPACE="")
        self.assertNotEqual(r.returncode, 0)
        self.assertIn("IMAGE_NAMESPACE", r.stderr)
        self.assertFalse(any(c[0] == "docker" for c in self.calls()))

    def test_publish_rejects_dirty_or_wrong_branch(self):
        for settings in [{"MOCK_DIRTY": " M example"}, {"MOCK_BRANCH": "wrong"}]:
            r = self.run_script("deploy/build/images.sh", "publish", "core", **settings)
            self.assertNotEqual(r.returncode, 0)
        self.assertFalse(any(c[0] == "docker" for c in self.calls()))

    def test_publish_cannot_overwrite_another_commit_tag(self):
        r = self.run_script("deploy/build/images.sh", "publish", "core", IMAGE_TAG="b" * 40)
        self.assertNotEqual(r.returncode, 0)
        self.assertFalse(any(c[0] == "docker" for c in self.calls()))

    def test_mavros_publish_uses_source_service_and_image_mapping(self):
        r = self.run_script("deploy/build/images.sh", "publish", "mavros")
        self.assertEqual(r.returncode, 0, r.stderr)
        build = next(c for c in self.calls() if c[1:3] == ["buildx", "build"])
        self.assertEqual(build[-1], str(ROOT / "src/mavros"))
        self.assertIn("ghcr.io/test-owner/drone-mavros:dev", build)
        self.assertIn(f"ghcr.io/test-owner/drone-mavros:{REVISION}", build)
        self.assertIn("linux/arm64", build)
        self.assertIn("--push", build)
        self.assertNotIn("--load", build)

    def test_vision_publishes_registry_base_before_app(self):
        r = self.run_script("deploy/build/images.sh", "publish", "vision")
        self.assertEqual(r.returncode, 0, r.stderr)
        builds = [c for c in self.calls() if c[1:3] == ["buildx", "build"]]
        self.assertEqual(len(builds), 2)
        self.assertIn(str(ROOT / "src/drone-vision/Dockerfile.base"), builds[0])
        self.assertIn(f"VISION_BASE_IMAGE=ghcr.io/test-owner/drone-vision-base:{REVISION}", builds[1])

    def test_pi_update_rejects_amd64_before_git_pull(self):
        r = self.run_script("deploy/update.sh")
        self.assertNotEqual(r.returncode, 0)
        self.assertFalse(any(c[0] == "git" and "pull" in c for c in self.calls()))

    def test_git_pull_failure_never_starts_containers(self):
        r = self.run_script("deploy/update.sh", MOCK_HOST_ARCH="aarch64", MOCK_GIT_PULL_FAILURE="1")
        self.assertNotEqual(r.returncode, 0)
        self.assertFalse(any(c[0] == "docker" and "up" in c for c in self.calls()))

    def test_bad_image_architecture_or_revision_never_starts_containers(self):
        for settings in [{"MOCK_IMAGE_ARCH": "amd64"}, {"MOCK_IMAGE_REVISION": "b" * 40}]:
            r = self.run_script("deploy/update.sh", MOCK_HOST_ARCH="aarch64", **settings)
            self.assertNotEqual(r.returncode, 0)
        self.assertFalse(any(c[0] == "docker" and "up" in c for c in self.calls()))

    def test_missing_private_bind_stops_before_image_pull(self):
        config = json.loads((self.path / "compose.json").read_text())
        config["services"]["cc-agent"]["volumes"] = [{"type": "bind", "source": str(self.path / "missing.env"), "target": "/app/.env"}]
        (self.path / "compose.json").write_text(json.dumps(config))
        r = self.run_script("deploy/update.sh", MOCK_HOST_ARCH="aarch64")
        self.assertNotEqual(r.returncode, 0)
        self.assertFalse(any(c[0] == "docker" and "pull" in c for c in self.calls()))

    def test_optional_update_adds_dependencies_without_build_or_provisioning(self):
        r = self.run_script("deploy/update.sh", "--service", "mavros", MOCK_HOST_ARCH="aarch64")
        self.assertEqual(r.returncode, 0, r.stderr)
        up = next(c for c in self.calls() if c[0] == "docker" and "up" in c)
        self.assertIn("mavros", up)
        self.assertIn("mavlink-router-controller", up)
        self.assertIn("--no-build", up)
        self.assertIn("never", up)
        self.assertFalse(any("build" in c or "save" in c or "load" in c or c[0] == "ssh" for c in self.calls()))

    def test_core_update_does_not_include_optional_profiles(self):
        r = self.run_script("deploy/update.sh", MOCK_HOST_ARCH="aarch64", COMPOSE_PROFILES="tailscale,vision,mavros")
        self.assertEqual(r.returncode, 0, r.stderr)
        up = next(c for c in self.calls() if c[0] == "docker" and "up" in c)
        self.assertNotIn("drone-tailscale", up)
        self.assertNotIn("drone-vision", up)
        self.assertNotIn("mavros", up)

    def test_check_only_has_no_update_side_effects(self):
        r = self.run_script("deploy/update.sh", "--check", MOCK_HOST_ARCH="aarch64")
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertFalse(any("pull" in c or "up" in c for c in self.calls()))

    def test_verification_waits_for_agent_socket(self):
        r = self.run_script("deploy/ship/verify-deployment.sh", "cc-agent", MOCK_SOCKET_DELAY="1")
        self.assertEqual(r.returncode, 0, r.stderr)
        probes = [c for c in self.calls() if "/run/drone/hw_manager.sock" in c]
        self.assertEqual(len(probes), 2)


if __name__ == "__main__":
    unittest.main()
