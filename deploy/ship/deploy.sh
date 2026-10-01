#!/usr/bin/env bash
# Optional SSH convenience: publish on WSL, execute Git/registry update on Pi.
set -Eeuo pipefail
# shellcheck source=deploy/lib/common.sh
source "$(dirname "$0")/../lib/common.sh"

DO_PUBLISH=true; TARGET=core; REMOTE=""; VERIFY_PUSHED=true
while (($#)); do
    case "$1" in
        --no-publish|--no-build) DO_PUBLISH=false; shift ;;
        --skip-pushed-check) VERIFY_PUSHED=false; shift ;;
        -h|--help) echo 'Usage: deploy.sh [--no-publish] [--skip-pushed-check] [service] [user@host]'; exit 0 ;;
        *@*|*.local|[0-9]*.[0-9]*) REMOTE="$1"; shift ;;
        *) TARGET="$(service_name "$1")"; shift ;;
    esac
done
registry_settings
load_env_file "$DEPLOY_ROOT/deploy/ship/target.env"
require_clean_branch
REVISION="$(git -C "$DEPLOY_ROOT" rev-parse HEAD)"
if [[ "$VERIFY_PUSHED" == true ]]; then
    upstream="$(git -C "$DEPLOY_ROOT" rev-parse --abbrev-ref '@{upstream}')" || die 'Configure a Git upstream and push the branch first'
    remote_name="${upstream%%/*}"
    git -C "$DEPLOY_ROOT" fetch "$remote_name"
    [[ "$(git -C "$DEPLOY_ROOT" rev-parse '@{upstream}')" == "$REVISION" ]] || die 'Push the current commit before deploying'
fi
if [[ "$DO_PUBLISH" == true ]]; then
    bash "$DEPLOY_ROOT/deploy/build/images.sh" publish "$TARGET"
fi
# Pin remote rollout to the commit tag even when publishing also made a dev tag.
if [[ -z "$REMOTE" ]]; then
    REMOTE="${PI_USER:-${TARGET_USER:-thong}}@${PI_HOST:-$(bash "$DEPLOY_ROOT/deploy/ship/resolve_target.sh" --quiet)}"
elif [[ "$REMOTE" != *@* ]]; then
    REMOTE="${PI_USER:-thong}@$REMOTE"
fi
[[ "$REMOTE" =~ ^[A-Za-z0-9_.-]+@[A-Za-z0-9_.:-]+$ ]] || die 'Invalid SSH target'
REMOTE_DIR="${TARGET_REMOTE_DIR:-\~/drone-edge}"
printf -v remote_command 'bash -s -- %q %q %q %q %q %q %q' \
    "$REMOTE_DIR" "$REGISTRY" "$IMAGE_NAMESPACE" "$REVISION" "$EXPECTED_BRANCH" "$TARGET" "$REVISION"
ssh -o BatchMode=yes "$REMOTE" "$remote_command" <<'REMOTE_SCRIPT'
set -Eeuo pipefail
remote_dir="$1"
if [[ "$remote_dir" == '~/'* ]]; then remote_dir="$HOME/${remote_dir:2}"; fi
cd "$remote_dir"
[[ -z "$(git status --porcelain)" ]] || { echo 'ERROR: Pi checkout is dirty' >&2; exit 1; }
[[ "$(git branch --show-current)" == "$5" ]] || { echo 'ERROR: Pi branch mismatch' >&2; exit 1; }
git pull --ff-only
export REGISTRY="$2" IMAGE_NAMESPACE="$3" IMAGE_TAG="$4" EXPECTED_BRANCH="$5"
bash deploy/update.sh --no-git-pull --service "$6" --expected-revision "$7"
REMOTE_SCRIPT
