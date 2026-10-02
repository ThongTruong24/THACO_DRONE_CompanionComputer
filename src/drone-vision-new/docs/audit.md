# Baseline and architecture audit

Baseline: `83f14620cf1aad362140a0ecf0dde1b96c06ec8b`; initial tree was clean.
Protected old implementation directories, old Vision, and root Compose remain
byte-for-byte unchanged. No production service was restarted or deployed.

## Effective router configuration

The production Dockerfile starts the native supervisor. `RouterSupervisor::start`
generates `/etc/mavlink-router/main.conf`; the child explicitly receives `-c` with
that path. The production Compose file does not mount the checked-in router config.
The generator emits TCP 5760 and UDP 14550/14541/14600 in both active and standby
configurations. Static `mavlink-router.conf` and an unused Jinja template include
UDP 14540, but the production generator does not. The legacy Python TCP proxy is
also not the production entrypoint. No old configuration was changed.

The real-router fixture uses the unchanged generator, passes an empty additional
configuration directory, and substitutes ephemeral ports for normal isolation.
An additional run used the exact generated production ports on the development
host. Initial audit and final native bridge both passed real bidirectional TCP/UDP
routing and the unchanged cc-agent GET roundtrip.

## Module map

| Module | Current implementation / runtime responsibility |
|---|---|
| cc-agent | Native C++ configuration transactions, metrics, MAVLink, HardwareRegistry and UDS hub; local UDP 14601 -> router 14600 |
| mavlink-router-controller | Native C++ supervisor, generated configuration, router child process and hotplug/watchdog; FC/SIYI UART, GCS, MAVROS, agent and TCP participants |
| camera-stream-controller | Native capture, NEON frame processing, standby frames, RTMP `/camera`; MediaMTX owns raw and annotated stream listeners |
| drone-networking | AP/STA orchestration, dnsmasq DHCP/DNS and mDNS; owns networking changes |
| drone-vision | Existing Python RTSP capture, YOLO model, annotation and RTMP `/yolo`; optional legacy ArUco source is not packaged as its supported mode |
| mavros | Optional ROS 2 Jazzy bridge, custom telemetry plugin, telemetry node and local logging socket |
| drone-vision-new | New isolated Python Vision pipeline plus generated-header C++ MAVLink TCP bridge; standalone Compose project |

Existing Vision's supported stream is `/camera -> inference -> RTMP /yolo ->
MediaMTX -> RTSP /yolo`. Root Compose currently overlays old application source,
entrypoint, config and model from the Pi checkout. The new standalone deployment
has no such overlays. Source/model belong to the final image.

## Ports

This is a source/config map, not a remote Pi socket inventory.

| Port | Owner | Direction / protocol | Source | Purpose / conflict risk |
|---|---|---|---|---|
| 14550 | Router | bind / bidirectional UDP | router_supervisor.cpp:317 | QGC; occupied; last-sender return address prevents safe sharing |
| 14541 | Router | loopback bind / bidirectional UDP | router_supervisor.cpp:329 | MAVROS; occupied |
| 14542 | MAVROS | local source / RX UDP | mavros/entrypoint.sh:4 | connects to 14541; occupied when enabled |
| 14600 | Router | loopback bind / bidirectional UDP | router_supervisor.cpp:323 | existing agent channel |
| 14601 | cc-agent | wildcard bind / source UDP | mavlink_core.cpp:78 | existing agent local socket; must be free for local fixture |
| 5760 | Router | bind / bidirectional TCP | router_supervisor.cpp:303 | existing dynamic TCP participants; new Vision uses its own connection |
| 14540 | Legacy configuration | intended UDP server | mavlink-router.conf:34 | absent from production generator; rejected path |
| 14551 | Legacy ArUco Vision | outbound UDP | vision_processor.py:36 | no corresponding active router listener found |
| 14602 | No current owner | unused UDP candidate | no repository references | no longer needed by TCP design |
| 14777 | Existing tests | UDP loopback bind | tests/test_mavlink_loopback.cpp:21 | test-only fixed listener |
| 1935 | MediaMTX | TCP listener / RTMP publishers | mediamtx.yml:14 | `/camera` and `/yolo`; new Vision does not bind it |
| 8554 | MediaMTX | TCP listener | mediamtx.yml:13 | RTSP camera/yolo readers |
| 8889 | MediaMTX | TCP listener | mediamtx.yml:15 | WebRTC HTTP signaling |
| 9997 | MediaMTX | loopback TCP listener | mediamtx.yml:10 | API |
| 53 | dnsmasq | UDP/TCP DNS listener | dnsmasq.conf:8 | AP DNS; interface-specific binding |
| 67/68 | DHCP server/clients | bidirectional UDP | dnsmasq DHCP defaults | AP address allocation |
| 5353 | Avahi / host mDNS | multicast UDP | networking entrypoint.sh:188 | existing-host detection |
| 22 | Host SSH | TCP listener / deployment clients | resolve_target.sh:65 | operator access |
| 8000/8001 | MediaMTX | UDP RTP/RTCP | v1.9.3 defaults | configured UDP RTSP transport |
| 8002/8003 | MediaMTX | multicast UDP RTP/RTCP | v1.9.3 defaults | configured multicast transport |
| 8189 | MediaMTX | bidirectional UDP | v1.9.3 defaults | WebRTC media |
| 8888 | MediaMTX | TCP listener | v1.9.3 defaults | HLS |
| 8890 | MediaMTX | bidirectional UDP listener | v1.9.3 defaults | SRT |
| ephemeral | new Vision | local TCP source | OS allocation | independent client connection; no fixed UDP port |

MediaMTX dependency defaults are from its
[versioned configuration](https://raw.githubusercontent.com/bluenviron/mediamtx/v1.9.3/mediamtx.yml).
ROS 2 DDS ports depend on middleware and participant allocation. Outbound client
sockets, build downloads and manual Tailscale operation are not fixed new listeners.

Upstream UDP Server endpoints return traffic to the last sender; the TCP server
creates independent dynamic participant endpoints. A valid heartbeat establishes
the component route. Unknown custom message IDs are forwarded by the tested real
daemon; endpoint clients validate them using their generated THACO dialect.
See [router behavior](https://github.com/mavlink-router/mavlink-router/blob/master/README.md#endpoints)
and [router parser](https://github.com/mavlink-router/mavlink-router/blob/2362c620f483cef1edd574fb962a373a288e4b9e/src/endpoint.cpp).

## Dialect and identity

Repository history has no canonical `thaco_common.xml` or generated Python dialect.
`/home/thong/Mavlink` is absent. The nearby firmware checkout contains a different
`thaco` dialect; it is not substituted. Existing generated headers have MAVLink
wire protocol 2.0 and THACO XML hash `-6877484802594583588`.

cc-agent uses `1/191`. MAVROS's custom broadcaster also originates component 191
(a pre-existing identity overlap; unchanged). Old Vision YAML lists 191, but its
supported YOLO worker does not originate MAVLink traffic. New default `1/192`
uses the existing `MAV_COMP_ID_ONBOARD_COMPUTER2` enum and has no other originating
use in the audited source. Real hardware/third-party camera identities require
operator validation before deployment.

`CC_CONFIG_GET` is ID 42102, request_id uint32 plus key[64]. Its response
`CC_CONFIG_VALUE` is ID 42107, matching request_id plus key[64] and value[128].
Neither message contains target system/component fields. The handler calls the
read-only `ConfigEngine::get`; replies are correlated by request_id/key and source
`1/191`, not imaginary target fields. A fresh fixture returns baudrate `921600`.
Runtime production values may differ, so the test does not assert live Pi settings.

`CC_TELEMETRY_VISION` ID 42013 is semantically appropriate for model, dimensions,
confidence, inference rate and detection count. It is distinct from transaction
messages. No new dialect definition was created or generated header edited.
