# Git/registry deployment and optional SSH execution

The normal workflow is `git push` + `make publish-core` on WSL, then
`make update` on Pi. See [full deployment guide](../README.md).

`deploy.sh` optionally performs both publication and remote execution:

```bash
make deploy PI_HOST=drone.local
make deploy-fast PI_HOST=drone.local
make deploy-mavros PI_HOST=drone.local
```

Before publication/update it checks the clean tree and configured branch. By
default it fetches the upstream and requires HEAD to be pushed already. It never
commits or pushes Git. `--skip-pushed-check` explicitly skips that upstream check.
`--no-publish` (`--no-build` compatibility alias) skips publication.

The remote checkout must already exist and have a clean tree, correct branch,
Git upstream, private `.env`/runtime configuration and registry read access.
SSH executes Git pull and `deploy/update.sh`; no source rsync or Docker archive
streaming occurs. Shell arguments are quoted individually and image tags are
pinned to the WSL commit SHA. Private files are never sent by this script.

`resolve_target.sh` retains mDNS/port-22/fallback-IP discovery for setup/debug and
SSH convenience. Private `target.env` can be copied from `target.example.env`.
`TARGET_REMOTE_DIR` chooses the remote checkout, with default `~/drone-edge`.
The registry/Git workflow itself does not require host discovery or SSH.

`verify-deployment.sh` is read-only and supports real Compose names, including
`mavros`. It checks container readiness/architecture, agent binary/sockets,
router endpoints/no broadcast loops, and relevant process/listener checks.
An absent FC is valid router Standby; live telemetry/video requires field testing.

`offline-deploy.sh` is an explicitly separate legacy fallback using local image
archives and Git-tracked-file rsync. It never copies private config or installs
netplan. It is invoked only by `make deploy-offline`, not standard deploy/update.
