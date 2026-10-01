# ARM64 build and GHCR publication

The implementation is `images.sh`; Makefile forwards to it. Development builds
run on WSL x86_64 with `drone-builder` and `--platform linux/arm64`.

| Purpose | Target | Output |
|---|---|---|
| Core local build | `make build-core` / `make build-all` | `--load`, configured GHCR-style local tags |
| Core publication | `make publish-core` / `make publish` | `--push`, full SHA + development tag |
| MAVROS | `make build-mavros` / `make publish-mavros` | `src/mavros` → `drone-mavros`, service `mavros` |
| Vision base | `make build-vision-base` / `make publish-vision-base` | `Dockerfile.base` → `drone-vision-base` |
| Vision publication | `make publish-vision` | Publish base first, then app with GHCR base build arg |

Set `REGISTRY=ghcr.io`, actual `IMAGE_NAMESPACE`, and `IMAGE_TAG` in private `.env`
or command-line Make variables. See [deployment configuration/authentication](../README.md).
Publish rejects dirty trees/wrong branches. Local builds allow dirty trees but
mark the revision label `SHA-dirty`; the Pi updater rejects these for normal release.

Vision's app Dockerfile requires `VISION_BASE_IMAGE`. Publication supplies
`ghcr.io/IMAGE_NAMESPACE/drone-vision-base:FULL_GIT_SHA`, which BuildKit resolves
from the registry. A local `make build-vision` also needs a published base;
`VISION_BASE_IMAGE` can explicitly choose a GHCR base. `make build-vision-base`
only loads a local verification artifact; it does not make that artifact visible
to the docker-container builder automatically. Apt OpenCV/GStreamer remains the
supported camera decoding path; Ultralytics is installed without replacing it.

Builds use QEMU if required. Check `docker buildx inspect drone-builder` and Docker
group permissions before building. The WSL setup script is provisioning, not a
required recurring build step. UART, USB cameras and Pi Wi-Fi are runtime inputs.

After publication, manifest/image configs must target linux/arm64; attestation
manifests are ignored. Revision labels must match the published commit. Existing
SHA tags are validated/reused rather than overwritten. No ARM64 build has been
claimed merely because static checks passed.

`cross_build.sh` is a separate optional camera-binary export tool. It writes
`src_native/bin/drone_camera_streamer`; it is not the GHCR build setup and is not
used by production Compose. Its compiler runs in an ARM64 stage under emulation,
not a true amd64 cross-toolchain. The Pi deployment requires no exported host binary.
