# Camera alias implementation and validation — 2026-10-02

All implementation changes are inside `src/drone-vision-new/`. QGC, generated
MAVLink definitions, old service implementations, root production Compose, and
the standalone deployment Compose were not changed. No deployment, registry
publication, commit, or push was performed.

## Identity audit

Companion baseline: `83f14620cf1aad362140a0ecf0dde1b96c06ec8b`.
QGC baseline: `0179e28163da799b875aef9dcc0076d187a06ce6`, clean detached checkout;
local `dev/tab_cc_telemetry` points to that commit.

Before implementation, searches covered named camera enums, raw numbers 100–105,
component assignments, both repositories' source/configuration, SIYI references,
deployment configuration, and the available previous generated router config.
Generated enum/message IDs, percentages, geometry, delays, and UI values were
distinguished from actual component producers.

| ID | Standard name | Companion producer/config allocation before this task | QGC producer allocation | Decision |
|---|---|---|---|---|
| 100 | CAMERA | None found | MockLink camera 1 | Avoid as default |
| 101 | CAMERA2 | None found | MockLink camera 2 | Avoid as default |
| 102 | CAMERA3 | None found | None; enum/recognition only | Repository-unused candidate |
| 103 | CAMERA4 | None found | None; enum/recognition only | Repository-unused candidate |
| 104 | CAMERA5 | None found | None; enum/recognition only | Repository-unused candidate |
| **105** | **CAMERA6** | **None found** | **None; enum/recognition only** | **Selected configurable default** |

Evidence in the sibling QGC repository:

- `src/Comms/MockLink/MockLinkCamera.cc:40,48` allocates CAMERA/CAMERA2.
- `src/MAVLink/QGCMAVLink.cc:12–17` names all six components without allocating them.
- `src/Camera/QGCCameraManager.cc:155–177` accepts camera heartbeats from 100–105
  after initial Vehicle connection completes.
- `src/Camera/QGCCameraManager.cc:568–589` alternates REQUEST_MESSAGE and
  REQUEST_CAMERA_INFORMATION for discovery.
- `src/Camera/VehicleCameraControl.cc:157–159` reads point/rectangle capability flags.
- `src/FlightMap/Widgets/PhotoVideoControl.qml:17,323,345` permits a tracking-only
  camera control and provides the tracking-enabled toggle.
- `src/FlyView/OnScreenCameraTrackingController.qml:21–34` normalizes radius by
  video width; `VehicleCameraControl.cc:2448–2467` targets the selected camera ID.
- `src/Camera/VehicleCameraControl.cc:2500–2506` requests message 275 at 500000 µs.

Companion SIYI configuration declares UART `/dev/ttyAMA0` and baud 115200 in
`deploy/setup/hardware/rpi4/hardware.env:11–12`. The router transports SIYI frames
without allocating a camera identity. `camera-stream-controller/camera.yaml`
declares a local camera index and RTMP publication, not a MAVLink camera component.
Primary cc-agent is 191; the optional MAVROS custom telemetry plugin also uses 191;
the existing new Vision primary is 192.

**Runtime limit:** this development host had no running drone containers and no
`/run/drone` or `/etc/mavlink-router` runtime directories. No Pi was contacted.
Repository-unused 105 is not a claim that external FC/SIYI hardware never uses 105.
Confirm live uniqueness before operating; configure another free ID if necessary.

## Implemented contract

| Behavior | Implementation |
|---|---|
| Primary identity | Default 1/192 unchanged; onboard-controller HEARTBEAT and CC_TELEMETRY_VISION remain primary |
| Camera alias | `mavlink.camera_component_id`, default 105; reject values outside 100..105 or equal to primary |
| Connection | One TCP 5760 client carries both identities; no additional socket or container |
| Heartbeats | Primary ONBOARD_CONTROLLER 18; alias CAMERA 30; both INVALID autopilot 8, modes 0, ACTIVE 4, approximately 1 Hz |
| Camera information 259 | Vendor THACO; model AgriDrone Vision Selector; point capability 512 only; unknown optics as NaN; firmware 0; current dimensions or 0 until known |
| Other info fields | Empty definition URI; lens, gimbal and camera_device_id 0 |
| Discovery | Commands 512 requesting 259 and 521; alias ACK then information, with one shared alias sequence counter |
| Targets | Matching system/alias or target 0 broadcast semantics; wrong explicit target ignored |
| Point 2004 | Finite x/y/radius in [0,1]; valid events enter Python Selection with a command generation; invalid values DENIED |
| Primary point commands | Commands addressed to 192 are not camera-alias requests and do not enter Selection |
| Stop 2010 | Immediately invalidates native selected state, sends clear event, ACKs; publisher removes highlight on its next frame |
| ACK identity | Source configured system/camera alias; destination original command sender, e.g. 255/190 |
| Radius | `radius_px = radius_norm * frame_width`; zero is one pixel per current dialect |
| Interval 511 / message 275 | Positive interval clamped to configured maximum rate, default 2 Hz; 0 selects default; −1 disables; invalid negative/nonfinite values denied |
| Scheduling | No catch-up bursts; repeated interval requests cannot bypass periodic rate cap; disabled initially; reconnect preserves a requested interval with idle state |
| Active status 275 | ACTIVE 1/POINT 1, normalized selected bbox center, width-normalized enclosing radius; IN_STATUS 4 plus RENDERED 2 flags |
| Idle status 275 | IDLE 0/NONE 0, unknown coordinates NaN; used after STOP, expiry, detection loss, or stale publication |
| Rectangle fields | NaN in point/idle modes; rectangle capability is not advertised |
| Selection semantics | Current detections re-evaluated within click TTL; no persistent object ID, prediction, or fabricated tracking identity |
| Stale protection | Old command generations discarded; default snapshot freshness 2 s and click TTL 5 s |
| Video | Manual `rtsp://<Pi>:8554/yolo`; never send VIDEO_STREAM_INFORMATION 269 |

Periodic status encoding and socket transmission run in the native transport
worker. Publication only enqueues a latest selection snapshot through nonblocking
IPC; inference does not wait for status/network I/O. Separate latest telemetry and
selection slots prevent one message type overwriting the other. Re-rendering
cached detection frames ensures STOP/expiry does not require another YOLO result.

No capture, recording, storage, zoom, focus, rectangle tracking, or automatic video
stream capability is advertised. Unsupported requests receive UNSUPPORTED rather
than fabricated successful functionality. The unrelated QGC CC telemetry field,
config UI, and single-source latch issues remain out of scope.

## Validation results

Tests were introduced before runtime integration. The initial selection tests
failed on diagonal radius handling/missing snapshot support; the camera tests
failed to compile because the protocol API did not yet exist.

| Check | Result |
|---|---|
| Native unit tests | 11/11 passed: codecs/config/queue/backoff plus camera protocol cases |
| Native TCP tests | 2/2 passed: both identities on one socket, fragmentation/coalescing/reconnect, bounded producer latency |
| Python unit tests | 17/17 passed on host and in the ARM64 image |
| TCP/native/Python IPC test | 1/1 passed; alias filtering, ACK source/target, invalid coordinates, reconnect clear |
| Real router with QGC dialect fixture | Passed using exact UDP 14550 and TCP 5760; configurable alias 104 also passed |
| Final ARM64 helper with same router/fixture | Passed with default alias 105; status intervals 0.509 s and 0.506 s in the recorded run |
| Primary generic telemetry | Fixture decoded 42013 from 1/192 with exact 640×360/count 1 test payload |
| Camera discovery | Both 512→259 and 521→259 passed; flag 512 only |
| Selection/status | Point reached Selection; yellow style; 2 Hz ACTIVE/POINT; loss IDLE; tall-frame width-radius rejection; STOP green style/IDLE; reconnect IDLE; −1 disabled |
| Video stream metadata | No 269 emitted; request 269 explicitly UNSUPPORTED |
| Actual QGC application/UI | **Not run:** no runnable QGC executable/AppImage was found |
| ARM64 build | Passed using `drone-builder`, linux/arm64, local load only |
| Packaged runtime | `--check`, model/config/headers, runtime shared libraries and 17 Python tests passed |
| Real Vision regression | ARM64 synthetic `/camera`→packaged YOLO→RTMP/yolo→3 decoded RTSP/yolo frames passed |
| Raw camera regression | Independent 640×360 `/camera` remained readable |
| Cached rendering | Actual OpenCV pixels proved yellow selection and green after STOP/expiry without another inference frame |

The router binary is the previously built real upstream revision
`2362c620f483cef1edd574fb962a373a288e4b9e`, MAVLink submodule
`052b8579f8aeb941f34cc9896af22cf1f38939b9`. No router source/image was changed or rebuilt.

The upstream router's `src/endpoint.cpp:437` tracks an expected packet sequence
per endpoint rather than per logical component. Mixing the primary and alias
sequence counters inflates that diagnostic's loss count; this fixture also starts
a fresh GCS encoder for each command. Validation checks decoded packets and their
timing, and does not claim a loss-free link from those endpoint counters.

QGC packet generation/decoding used separate `all` headers generated from its
configured dependency `ThongTruong24/agridrone-mavlink`, resolved main revision
`181947bb1c76f53076b29a0298f2238a37bb91a4`. The fixture target has no Raspberry-header
fallback. Camera information CRC 92 and tracking status CRC 126 agree. The fixture
reproduces the current QGC camera command sequence; it does not instantiate Qt's
VehicleCameraControl or prove a GUI interaction.

Local image: `ghcr.io/thongtruong24/drone-vision-new:camera-alias-local-test`.
Inspected image ID:
`sha256:d5adfb342441ff9c0a7730130a4142458bad25dc7cb3963f04af6e9242fe3794`.
Architecture: `linux/arm64`. Revision label:
`83f14620cf1aad362140a0ecf0dde1b96c06ec8b-camera-alias-dirty`.
This is a local image ID, not a published registry digest.

The Vision smoke test used test-only 320×240 frames, imgsz 160, inference 1 Hz,
publication 5 Hz, and 60 s stale allowance under QEMU. Production defaults remain
640×360, imgsz 320, inference 5 Hz, publication 20 Hz, stale allowance 2 s. Successful
emulation does not establish production Pi throughput.

## Changed files and scope verification

All paths below are relative to `src/drone-vision-new/`:

- Native: `native/bridge.hpp`, `native/bridge.cpp`, `native/camera_protocol.cpp`
  (new), `native/mavlink_bridge.cpp`, `native/CMakeLists.txt`.
- Python: `scripts/configuration.py`, `scripts/mavlink_client.py`,
  `scripts/target_selection.py`, `scripts/yolo_streamer.py`, `scripts/main.py`.
- Packaging: `config/drone.yaml`, `Dockerfile`.
- Tests: `tests/test_camera_protocol.cpp` (new), `tests/test_target_selection.py`,
  `tests/test_runtime.py`, `tests/test_tcp_transport.cpp`,
  `tests/tcp_ipc_integration.py`, `tests/container_smoke.py`,
  `tests/qgc_camera_packets.cpp` (new), `tests/qgc_camera_protocol.py` (new),
  `tests/arm_bridge.sh` (new).
- Documentation: `README.md`, `docs/system.puml`, `docs/sequence.puml`,
  `docs/validation.md`, this new `docs/camera-alias-validation.md`.

A before/after content digest checked all tracked and nonignored untracked files
outside the permitted module: **835 Companion files** and **3686 QGC files** were
unchanged. Both HEADs remained fixed. The pre-existing changes to root docs and
old `src/drone-vision/Dockerfile.base`, plus the untracked standalone deployment,
were preserved. No old container image was rebuilt.

The existing primary read-only GET/VALUE roundtrip also passed with the ARM64
helper: request `0x5a170001` / key `telemetry.fc.baudrate`, source 1/192, response 1/191,
fixture value 921600; observed enqueue-to-response 70.184 ms. This checks unchanged
primary behavior and is not a Pi latency benchmark or configuration reading.

## Reproduce

Run from the Companion repository on an isolated development host. First generate
QGC's current configured `all` dialect in a temporary directory using the official
pymavlink generator; record the dependency commit. This audit used the official
generator in the adjacent FC firmware checkout, with schema validation skipped
because validation dependencies were unavailable; no XML/header was edited.

```bash
git clone --depth 1 https://github.com/ThongTruong24/agridrone-mavlink.git /tmp/vision-qgc-dialect
git -C /tmp/vision-qgc-dialect rev-parse HEAD
# MAVLINK_SOURCE is the directory containing the official pymavlink package.
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH="$MAVLINK_SOURCE" \
  python3 "$MAVLINK_SOURCE/pymavlink/tools/mavgen.py" \
  --lang C --wire-protocol 2.0 --no-validate --output /tmp/vision-qgc-headers \
  /tmp/vision-qgc-dialect/message_definitions/v1.0/all.xml

cmake -S src/drone-vision-new/native -B /tmp/vision-camera-build \
  -DQGC_DIALECT_INCLUDE_DIR=/tmp/vision-qgc-headers/all \
  -DQGC_DIALECT_REVISION="$(git -C /tmp/vision-qgc-dialect rev-parse HEAD)"
cmake --build /tmp/vision-camera-build -j2
ctest --test-dir /tmp/vision-camera-build --output-on-failure
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover \
  -s src/drone-vision-new/tests -p 'test_*.py' -v
PYTHONDONTWRITEBYTECODE=1 python3 src/drone-vision-new/tests/tcp_ipc_integration.py /tmp/vision-camera-build
PYTHONDONTWRITEBYTECODE=1 python3 src/drone-vision-new/tests/qgc_camera_protocol.py \
  --router-binary /absolute/path/to/mavlink-routerd --build-dir /tmp/vision-camera-build
```

The fixture refuses occupied TCP 5760/UDP 14550/14541/14600 listeners. It uses a
temporary config rendered through unchanged RouterSupervisor and an empty extra
config directory. Its GCS sender 255/190 initiates UDP peer learning. Only the new
helper, real router, and local test endpoint are involved; no live FC/Pi is needed.

For the final image, build/load only the new Dockerfile using the README's ARM64
command, choosing the camera-alias-local-test tag. Then repeat the protocol fixture
with `--bridge-executable src/drone-vision-new/tests/arm_bridge.sh`. The wrapper
has no bind mounts; it executes the packaged AArch64 helper with the isolated
host's router. `VISION_CAMERA_TEST_IMAGE` can select another local new-image tag.

For `tests/container_smoke.py`, create an isolated new-image container with
`--network none`, copy the test script and test-only AArch64 MediaMTX binary to
`/tmp`, and run the script. No source bind mount or old service container is needed.

## Remaining manual Pi/QGC checks

1. Inspect live HEARTBEAT identities, including SIYI/external cameras, and confirm
   alias 105 is unused; configure another free 100..105 ID if required.
2. Confirm FC system ID equals configured system ID, MAVLink2 is active, QGC's
   Vehicle finishes initial connection, and QGC initiates unicast UDP 14550 learning.
3. In a runnable current QGC application, verify discovery/model name, select the
   Vision camera alias, enable native tracking, and set manual RTSP/yolo.
4. Click video corners/center with fit/letterboxing, observe yellow selected bbox
   and native point status overlay; verify actual command target is the alias.
5. Test STOP, five-second TTL, disappearance, camera standby, and TCP reconnect;
   verify green/no-selection feedback and idle status without another inference result.
6. Confirm manual RTSP stays configured, raw `/camera` remains healthy, and the old
   Vision publisher has released `/yolo`. Measure actual Pi CPU, memory, frame rates,
   video latency, and 2 Hz status intervals at production defaults.

Actual GUI success and live component uniqueness are not claimed by local fixtures.
