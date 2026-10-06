# CLAUDE.md

Always use Context7 when I need library/API documentation, code generation, setup or configuration steps without me having to explicitly ask.

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

@AGENTS.md
@PROJECT_CONTEXT.md

The imported files are canonical. **Always read `PROJECT_CONTEXT.md` first** (and `PROJECT_CONTEXT_DEEP.md` for protocol byte layouts / refactor history before any large refactor or deep debug). `AGENTS.md` indexes the mandatory rules under `.agents/rules/`. This file adds the orientation an agent needs on top of those; it does not restate them.

Most repo docs are written in Vietnamese — match that language when editing a doc that is already Vietnamese; code, identifiers, and commit messages stay in English.

## What this repo is

The **Drone Edge companion computer** for the THACO AgriDrone — a set of 6 independent C++20/Docker micro-services that run on a Raspberry Pi 5 (`thong.local`), build on WSL2, and deploy over SSH. This is the drone-side peer of the QGroundControl fork (`THACOGroundControl`); the two meet over MAVLink. Custom MAVLink messages (`CC_TELEMETRY_*`, THACO IDs like 42014) are shared with QGC through the same MAVLink fork.

## Build, test, deploy (from WSL)

`make help` lists every target. The ones you'll use most:

```bash
make build-native      # configure+build all native C++ into build/ (Release, BUILD_TESTING=ON)
make test              # build-native, then ctest --output-on-failure (the full 33-test suite)
make build-all         # cross-build 5 core ARM64 Docker images via buildx
make deploy            # build ARM64 -> ssh/scp to Pi -> load image -> recreate containers
make deploy-fast       # same, skipping the build (image already present)
make ps / make doctor  # container status / sequential auto-diagnostics on the Pi
make logs-<svc>         # follow a service's logs on the Pi (cc-agent, networking, camera-rtsp, ...)
```

Run a **single native test** against the already-built tree (don't re-run the whole suite while iterating):

```bash
cd build && ctest -R HardwareManagerServerTest --output-on-failure   # by name/regex
cd build && ctest -R IpcHardwareRouter --output-on-failure           # integration (real UDS)
```

Deploy targets (`make deploy`, `make deploy-<svc>`) resolve the Pi via `deploy/ship/resolve_target.sh` (zero-conf mDNS → IP fallback); override with `PI_HOST`/`PI_USER` in `.env`. **There is one deployment implementation — `deploy/ship/deploy.sh`. Never add a second.** Per-service deploy recreates only that container.

Only three modules are in the native CMake build (`CMakeLists.txt`): `mavlink-router-controller`, `cc-agent`, `camera-stream-controller`, plus the root `tests/` integration suite. `drone-vision` (Python/YOLO) and `mavros` (ROS 2) are Docker-only and gated behind Compose profiles `vision` / `mavros`.

## Architecture: the big picture

Six services (`src/<module>/`, each its own container, `README.md`, and `docs/*.puml`). Full topology table with ports/sockets is in `PROJECT_CONTEXT.md` §2–3; the service-boundary contract is `.agents/rules/00-core.md`. What matters across files:

- **cc-agent is the single hardware authority.** It is the only component allowed to scan `/dev/tty*`, validate character devices, and reserve/release serial ports, via `HardwareRegistry` behind `/run/drone/hw_manager.sock`. The old standalone `hardware-manager` module was **merged into cc-agent** — `make build-hw` now just aliases `build-cc-agent`; don't reintroduce a separate hardware service. cc-agent also runs the 2-phase config transaction engine with a 30s heartbeat rollback guard, and is a MAVLink telemetry provider (CompID 191).
- **Services talk over Unix domain sockets + low-latency UDP**, not shared files: `/run/drone/{hw_manager,cc_agent,router}.sock` for control; UDP `:14550` (GCS), `:14600` (cc-agent), `:14541` (MAVROS) for MAVLink.
- **mavlink-router-controller must never crash when the FC is absent.** With no Cube Orange on `/dev/ttyAMA4` it starts in **Graceful Standby Mode** (UDP-only, ports open), and reconfigures live via `/run/drone/router.sock` when hardware appears.
- **Networking ownership is exclusive.** Only `drone-networking` creates/configures interfaces (AP-STA, `dnsmasq` DHCP, mDNS). Don't put host-networking logic in other services.

## Non-negotiable invariants (enforced by the rules, easy to violate)

Full statements in `PROJECT_CONTEXT.md` §1 and `.agents/rules/`:

- **MAVLink single source of truth** — custom message definitions are edited ONLY at `$HOME/Mavlink/custom/thaco_common.xml`, then propagated with `$HOME/Mavlink/sync_all.sh`. Never hand-edit generated headers (under any module's `mavlink/`). (`.agents/rules/02-mavlink.md`)
- **No broadcast on 14550** — the router runs `network_mode: host`; broadcast endpoints (e.g. `192.168.10.255`) on 14550 cause a telemetry-amplification loop. (`02-mavlink.md`)
- **All C++ is C++20 native, tested with GoogleTest/CTest.** New logic lands with tests; keep `make test` green (33/33).
- **Doc-sync governance** — changes to a module must keep its `README.md` and UML (`docs/sequence.puml`, `docs/system.puml`) in sync, and keep ports/IPs/service names/Makefile targets consistent across every layer. (`.agents/rules/06-doc-sync-and-rule-governance.md`)
- **Ask the user before editing any file under `.agents/rules/`.** (`06-doc-sync-and-rule-governance.md`)

## Hardware / network endpoints (develop against these)

UART4 `/dev/ttyAMA4` = FC @921600 · UART0 `/dev/ttyAMA0` = SIYI @115200 · `eth0` static `192.168.144.11/24` (SIYI link) · AP `uap0` = `AP_DRONE` @ `192.168.10.1` · GCS `thong.local:14550` · RTSP `rtsp://thong.local:8554/{camera,yolo}` · WebRTC `http://thong.local:8889/camera`.
