#!/usr/bin/env bash
# One build implementation: --load for local work, --push for GHCR releases.
set -Eeuo pipefail
# shellcheck source=deploy/lib/common.sh
source "$(dirname "$0")/../lib/common.sh"

MODE="${1:-}"; TARGET="${2:-core}"
[[ "$MODE" == build || "$MODE" == publish ]] || die 'Usage: images.sh build|publish [core|service|vision-base]'
registry_settings
TARGET="$(service_name "$TARGET")"
cd "$DEPLOY_ROOT"
REVISION="$(git rev-parse HEAD)"
if [[ "$MODE" == publish ]]; then
    require_clean_branch
    if [[ "$IMAGE_TAG" =~ ^[0-9a-f]{7,40}$ && "$REVISION" != "$IMAGE_TAG"* ]]; then
        die 'A Git SHA IMAGE_TAG must identify the current commit'
    fi
elif [[ -n "$(git status --porcelain)" ]]; then
    REVISION="${REVISION}-dirty"
fi
[[ "$(uname -m)" == x86_64 ]] || die 'Build/publish runs on the x86_64 development machine, not the Pi'
BUILDER="${BUILDER:-drone-builder}"
[[ "$BUILDER" == drone-builder ]] || die 'Use BUILDER=drone-builder'
docker info >/dev/null
if ! docker buildx inspect "$BUILDER" >/dev/null 2>&1; then
    docker buildx create --name "$BUILDER" --driver docker-container
fi
docker buildx inspect "$BUILDER" --bootstrap >/dev/null

build_image() {
    local svc="$1" context dockerfile ref sha_ref base_ref
    local args=()
    context="$(service_source "$svc")"
    dockerfile="$context/Dockerfile"
    [[ "$svc" != drone-vision-base ]] || dockerfile="$context/Dockerfile.base"
    ref="$REGISTRY/$IMAGE_NAMESPACE/$(service_image "$svc"):$IMAGE_TAG"
    sha_ref="$REGISTRY/$IMAGE_NAMESPACE/$(service_image "$svc"):$REVISION"
    if [[ "$svc" == drone-vision ]]; then
        # Publish the base first. Local app builds require an already-published base.
        base_ref="${VISION_BASE_IMAGE:-$REGISTRY/$IMAGE_NAMESPACE/drone-vision-base:$(git rev-parse HEAD)}"
        [[ "$base_ref" == "$REGISTRY/$IMAGE_NAMESPACE/"* ]] || die 'VISION_BASE_IMAGE must be published in the configured GHCR namespace'
        verify_registry_image "$base_ref"
        args+=(--build-arg "VISION_BASE_IMAGE=$base_ref")
    fi
    if [[ "$MODE" == publish ]] && docker buildx imagetools inspect --raw "$sha_ref" >/dev/null 2>&1; then
        verify_registry_image "$sha_ref" "$REVISION"
        echo "Reuse published commit image: $sha_ref"
        if [[ "$ref" != "$sha_ref" ]]; then
            docker buildx imagetools create --tag "$ref" "$sha_ref"
            verify_registry_image "$ref" "$REVISION"
        fi
        return
    fi
    args+=(--builder "$BUILDER" --platform linux/arm64 --file "$dockerfile"
        --label "org.opencontainers.image.revision=$REVISION" --tag "$ref")
    if [[ -n "${IMAGE_SOURCE_URL:-}" ]]; then
        [[ "$IMAGE_SOURCE_URL" =~ ^https://github.com/[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$ ]] || die 'IMAGE_SOURCE_URL must be a GitHub repository HTTPS URL without credentials'
        args+=(--label "org.opencontainers.image.source=$IMAGE_SOURCE_URL")
    fi
    if [[ "$MODE" == publish ]]; then
        [[ "$ref" == "$sha_ref" ]] || args+=(--tag "$sha_ref")
        args+=(--push)
    else
        args+=(--load)
    fi
    echo "$MODE $svc -> $ref (linux/arm64)"
    docker buildx build "${args[@]}" "$context"
    if [[ "$MODE" == publish ]]; then
        verify_registry_image "$sha_ref" "$REVISION"
        [[ "$ref" == "$sha_ref" ]] || verify_registry_image "$ref" "$REVISION"
    else
        docker image inspect "$ref" | python3 "$DEPLOY_ROOT/deploy/build/validate_image.py" config "$REVISION"
    fi
}

if [[ "$TARGET" == core ]]; then
    for service in "${CORE_SERVICES[@]}"; do build_image "$service"; done
else
    if [[ "$MODE" == publish && "$TARGET" == drone-vision ]]; then build_image drone-vision-base; fi
    build_image "$TARGET"
fi
