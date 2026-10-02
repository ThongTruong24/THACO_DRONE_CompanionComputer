# Implementation and validation report

Historical report for the initial primary 192-only implementation. The subsequent
camera alias work and current results are in [camera-alias-validation.md](camera-alias-validation.md).
The original real-router runner opened14550 but its UDP participant actually used
14541. The newer fixture explicitly proves traffic through 14550.

This report covers the local implementation and tests. No image was published,
no Pi was contacted or deployed, and no Git commit or push was made. Existing
production containers were not restarted. Tests used isolated development-host
fixtures, temporary agent state, and the new local ARM64 image.

## 1. Baseline SHA

`83f14620cf1aad362140a0ecf0dde1b96c06ec8b`. The initial checkout was clean;
HEAD remains at this baseline. All additions are working-tree changes.

## 2. TCP router evidence

The unchanged `RouterSupervisor::generate_config` emits `TcpServerPort = 5760`
in active and standby configurations. The supervisor launches the daemon with
the generated configuration. UDP 14540 is absent from that generated file.

The real daemon was built from the upstream repository referenced by the unchanged
router Dockerfile, at revision `2362c620f483cef1edd574fb962a373a288e4b9e`, with
MAVLink submodule `052b8579f8aeb941f34cc9896af22cf1f38939b9`. The fixture renders
configuration through the unchanged C++ generator, supplies an empty extra-config
directory, and starts the actual daemon. This is not a router mock or a test
against the legacy Python proxy.

Bidirectional routing passed with ephemeral ports and with exact local production
ports TCP 5760 and UDP 14550/14541/14600. A UDP participant heartbeat arrived at
the new TCP transport, and a generated THACO GET from that transport arrived at
the UDP participant. The unchanged agent received that request through UDP 14600
and its UDP 14601 reply reached the independent TCP participant.

The production Dockerfile clones unpinned upstream source. This revision proves
the implementation built for these fixtures; it does not identify the binary
already running on a Pi.

## 3. MAVLink dialect strategy

Strategy B: a new native C++ helper uses the same generated MAVLink 2
`thaco_common` C headers as cc-agent. The repository/history and local audit did
not find canonical `thaco_common.xml` or generated Python bindings. The nearby
firmware dialect is different and was not substituted.

Generated headers are read-only build inputs. No dialect definitions, generated
headers, packet layout, or CRC constants were edited or hand-coded. Generated
pack/decode functions and the generated parser handle complete MAVLink frames.

## 4. sysid / compid

Configurable defaults: `1/192`; 192 is `MAV_COMP_ID_ONBOARD_COMPUTER2` and has no
other originating use in the audited repository. Component 191 is rejected by
the new configuration. Existing cc-agent uses `1/191`; a MAVROS custom broadcaster
also uses 191, a pre-existing overlap that this change does not alter. Live
third-party participants still require an operator identity check.

## 5. Final architecture

```text
Flight controller <-- UART --> existing mavlink-routerd <-- UDP 14550 --> QGC
                                      |
                                      +<-- UDP 14541 --> existing MAVROS (optional)
                                      +<-- UDP 14600 / 14601 --> unchanged cc-agent
                                      |
                               TCP 127.0.0.1:5760
                                      |
existing camera/MediaMTX               |
          | RTSP /camera              |
          v                           v
  +----------------------- drone-vision-new ------------------------+
  | capture worker       native C++ worker <--> NDJSON pipe worker  |
  |       |                                          |             |
  | latest frame --> YOLO inference <-------- selection/telemetry   |
  |                       |                                        |
  |              latest annotated frame --> publish worker         |
  +--------------------------------------------|-------------------+
                                               | RTMP /yolo
                                               v
                                    existing MediaMTX
                                               | RTSP /yolo
                                               v
                                              QGC
```

Capture, inference, publication and MAVLink socket I/O run independently. Python
pipe I/O has its own worker. Inference only enqueues telemetry. Frame mailboxes
retain one item, requests/events have fixed bounds, and telemetry is latest-only.
The TCP source port is OS-assigned; no new fixed UDP listener is needed.

Disconnect resets the native stream parser, drops pending transmissions, clears
selection and reconnects with bounded exponential backoff. Vision continues when
the router is unavailable. Selection expires after five seconds and is recomputed
from image coordinates and current detections, rather than persisting a box index.

## 6. Files added

Thirty files: 29 in the new module and one standalone deployment file.

```text
deploy/drone-vision-new/compose.yaml
src/drone-vision-new/
  Dockerfile
  README.md
  entrypoint.sh
  config/drone.yaml
  docs/audit.md
  docs/sequence.puml
  docs/system.puml
  docs/validation.md
  native/CMakeLists.txt
  native/bridge.hpp
  native/bridge.cpp
  native/mavlink_bridge.cpp
  scripts/configuration.py
  scripts/main.py
  scripts/mavlink_client.py
  scripts/target_selection.py
  scripts/bbox_renderer.py
  scripts/yolo_streamer.py
  tests/test_mavlink_codec.cpp
  tests/test_tcp_transport.cpp
  tests/test_target_selection.py
  tests/test_renderer.py
  tests/test_runtime.py
  tests/tcp_ipc_integration.py
  tests/router_probe.cpp
  tests/packet_fixture.cpp
  tests/render_baseline_config.cpp
  tests/run_real_integration.py
  tests/container_smoke.py
```

## 7. Files modified

Only root `README.md` and `PROJECT_CONTEXT.md`, to document this optional module.
Git checks confirm no changes or untracked additions under protected old module
directories, `.agents/rules`, or root Compose. Existing model and generated headers
are copied during the new image build, without editing their sources.

## 8. Unit test results

**18/18 passed**: six native tests and twelve Python tests. Python tests also
passed inside the final ARM64 image. True unit tests have no network dependency.

Coverage includes generated encoding/decoding, source identity, message/payload
fields, corrupt CRC/truncated-frame rejection, parser fragmentation/coalescing,
exact command targets, bounded/latest queues, backoff reset, configuration/model
validation, frame expiry, geometry/radius/ties, selection expiry/reordering and
renderer styles.

Tests were written before application integration. The initial native build and
Python discovery failed because the implementation files/modules did not exist;
implementation was then added to satisfy these checks. Transport shutdown and
live reconnect behavior are verified in the separate socket tests below.

## 9. TCP transport tests

**2/2 native TCP tests and 1/1 Python/native IPC integration test passed.**

Local servers prove TX and RX, split reads, coalesced messages, exact fields,
reconnect after an incomplete frame, parser reset, client restart, bounded
producer latency when the endpoint refuses connections, event timeout and clean
shutdown. The IPC test also sends generated tracking commands, verifies target
filtering and accepted/denied acknowledgments, and checks selection clear on
disconnect and command expiry. No manual MAVLink serialization is used by the
Python fixture; its native packet fixture uses the same generated headers.

## 10. Real router integration result

**Passed.** Actual TCP-to-UDP and UDP-to-TCP packet delivery was asserted, rather
than inferring routing from an open socket. Normal fixtures remap ports for
isolation; the additional `--production-ports` run exercised TCP 5760 directly.

## 11. cc-agent read-only roundtrip result

**Passed**, including a final run with the helper inside the ARM64 image.

| Field | Transmitted request | Received response |
|---|---|---|
| Name / message ID | `CC_CONFIG_GET` / 42102 | `CC_CONFIG_VALUE` / 42107 |
| sysid / compid | 1 / 192 | 1 / 191 |
| request_id | `0x5a170001` (1511456769) | Same |
| Key | `telemetry.fc.baudrate` | Same |
| Value | No value field | `921600` |
| Target sysid/compid | No target fields in this message | No target fields in this message |
| Decode | Generated dialect encoder | Generated dialect parser/decoder |

The final exact-port run recorded native roundtrip **5.847 ms**. The Python pipe
client using the final container's AArch64 helper recorded **70.288 ms**, measured
from request enqueue to decoded response delivery. These are single local samples,
not Pi latency benchmarks.

Final helper events recorded TX timestamp `1790905114.2738955` and RX timestamp
`1790905114.2776146`, with endpoint `127.0.0.1:5760` and the exact fields above.
No transaction begin/set/apply/confirm/rollback or flight command was issued.
The fixture asserts that the agent's temporary configuration directory and
telemetry root remain empty after GET.

## 12. Vision tests

The ARM64 runtime smoke test passed with the real packaged YOLO model:

```text
synthetic RTMP /camera -> MediaMTX RTSP /camera -> YOLO inference
    -> annotation -> RTMP /yolo -> MediaMTX RTSP /yolo -> 3 decoded frames
```

Raw `/camera` remained readable at 640x360. Separate actual OpenCV pixel checks
proved normal green and selected yellow boxes and that rendering preserves its
input frame. Unit tests check the thicker selected outline and `[SELECTED]` label.
No router ran in this fixture: socket reconnect did not stop inference/publication.

The first smoke attempt timed out under QEMU with the packaged two-second stale
allowance. The passing fixture used test-only 320x240 frames, imgsz 160, inference
1 Hz, output 5 Hz, stale allowance 60 seconds and a longer deadline. Production
defaults remain 640x360, imgsz 320, inference 5 Hz, output 20 Hz, two-second stale
allowance and two Torch threads. This proves pipeline functionality under
emulation; it does not prove those production rates on a Pi.

The old unchanged Vision suite separately produced **7 passed, 1 failure, 4
errors**, from pre-existing API/signature mismatches. Those baseline failures were
not repaired and are not included in the new module's passing unit-test count.

## 13. ARM64 build result

**Passed** using `docker buildx build --builder drone-builder --platform
linux/arm64 --load`. Only the new runtime image was built. The initial builder
attempt lacked `make`; the new Dockerfile's build-stage dependencies were fixed
and the final build completed.

- Local tag: `ghcr.io/thongtruong24/drone-vision-new:local-test`.
- Image ID: `sha256:fd671652228cdbba66d4ae48e659458ef42bad53d1b1d798c329923a195fd874`.
- Architecture: `linux/arm64`, independently confirmed with `docker image inspect`.
- Extracted helper ELF machine: `AArch64`, confirmed with `readelf -h`.
- Revision label: `83f14620cf1aad362140a0ecf0dde1b96c06ec8b-dirty`.
- Size: 3,013,783,165 bytes uncompressed; local image ID is not a published digest.
- Final image `--check`, native generated-codec self-test and twelve Python unit
  tests passed without network access.

Only unchanged native router/agent code was compiled as test fixtures; their
container images were not built or replaced. QEMU/binfmt was enabled on the local
development host to execute the new ARM64 image.

## 14. Final image contents

The image owns `/app/scripts`, `/app/config/drone.yaml`, `/app/entrypoint.sh`,
`/usr/local/bin/mavlink-bridge` and `/app/models/yolo26n.pt`. Python, Torch,
Ultralytics, OpenCV and GStreamer come from the pinned dependency image:

`ghcr.io/thongtruong24/drone-vision@sha256:9e4f40d9c8da4afee7e3d4e711d04eae11dbfb6940d8e93ca94f05a84007d2d6`.

Inherited application directories are replaced with the new implementation.
The model is 5,544,453 bytes; its extracted image checksum matches the baseline:
`9b09cc8bf347f0fc8a5f7657480587f25db09b34bf33b0652110fb03a8ad4fef`.
Python unit tests are also packaged for optional offline verification.

Compose validation confirms exactly one service, host networking, ARM64, no
`build`, no `depends_on`, and no source/model/default-config bind mounts. The only
bind is read-only `/run/drone` for existing camera standby status.

## 15. Raspberry deployment commands

These commands were **not executed**. After separate operator publication of a
chosen tag, replace `<tag>` and run from the existing stack directory:

```bash
export VISION_NEW_IMAGE='ghcr.io/thongtruong24/drone-vision-new:<tag>'
docker pull "$VISION_NEW_IMAGE"
docker compose --profile vision stop drone-vision
docker compose -f deploy/drone-vision-new/compose.yaml up -d --no-build --pull never
docker compose -f deploy/drone-vision-new/compose.yaml logs --tail 50
```

Only the new image and standalone Compose file are needed for the new application.
The old stack continues supplying router, agent, camera/MediaMTX and networking.
Existing MAVROS remains optional. The new application does not require a Pi source
checkout; the Compose file can be transferred on its own.

## 16. Rollback commands

```bash
docker compose -f deploy/drone-vision-new/compose.yaml down
docker compose --profile vision start drone-vision
```

Stop one publisher before starting the other: existing MediaMTX rejects duplicate
publishers for `/yolo`. No old service configuration migration is needed.

## 17. Remaining risks and limits

- Actual Pi binaries, live component identity uniqueness, physical camera input,
  CPU/memory use and output rates still need operator runtime validation. This
  work did not access the Pi, FC or QGC.
- The source-level router upstream is unpinned; its tested revision is recorded
  above. Deployed binary provenance is not inferred from that clone.
- Target selection is a TTL-limited image-space click, not persistent object
  tracking; it may switch detections as objects move. Radius is implemented as a
  fraction of frame diagonal, with zero allowing unrestricted nearest selection.
- Commands must explicitly target the configured component. The onboard-computer
  heartbeat is not a complete camera discovery/capability protocol, so native QGC
  camera tracking controls are not guaranteed to appear.
- The inherited ML dependency image is large. CPU inference throughput and the
  default 1.5 GiB Compose limit were not benchmarked on Raspberry hardware.
- The canonical dialect source is absent locally. This helper reuses existing
  generated headers, and future dialect changes must follow project generation
  rules rather than modifying those headers.
- Existing old Vision test failures and the old MAVROS component-191 overlap remain
  baseline issues outside this implementation.

## Reproduction

Unit, socket, router/agent and image-build commands are in the
[module README](../README.md#tests). To use the final ARM64 helper in the real
roundtrip fixture, supply a wrapper as `--bridge-executable`:

```bash
#!/usr/bin/env bash
exec docker run --rm -i --init --network host --platform linux/arm64 \
  --entrypoint /usr/local/bin/mavlink-bridge \
  ghcr.io/thongtruong24/drone-vision-new:local-test "$@"
```

The image streaming smoke test needs a test-only ARM64 MediaMTX v1.9.3 executable
at `/tmp/mediamtx` and the test harness at `/tmp/container_smoke.py`. A local
isolated invocation is:

```bash
docker run --rm --platform linux/arm64 --network none --cpus 8 --memory 2g \
  -v /absolute/path/to/mediamtx:/tmp/mediamtx:ro \
  -v "$PWD/src/drone-vision-new/tests/container_smoke.py:/tmp/container_smoke.py:ro" \
  --entrypoint python3 ghcr.io/thongtruong24/drone-vision-new:local-test \
  /tmp/container_smoke.py
```

These are test-tool mounts; application source/config/model are read from the
image. Local build and smoke logs were retained under
`/tmp/drone-vision-new-validation.YBDnJK/` for this session, but are not deployment
inputs or committed repository artifacts.
