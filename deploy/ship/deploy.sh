#!/usr/bin/env bash
# ==============================================================================
# deploy/ship/deploy.sh - High-Performance Optimized Deploy lên Raspberry Pi
# ==============================================================================

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
REMOTE_DIR="~/drone-edge"
CACHE_DIR="${WORKSPACE_DIR}/.drone_build_cache"
mkdir -p "$CACHE_DIR"

# Nạp cấu hình từ .env nếu có
ENV_FILE="${WORKSPACE_DIR}/.env"
if [ -f "$ENV_FILE" ]; then
    export $(grep -v '^#' "$ENV_FILE" | xargs)
fi

normalize_pi_host() {
    local host="$1"
    if [[ "$host" != *.* ]] && [[ "$host" != *:* ]]; then
        echo "${host}.local"
    else
        echo "$host"
    fi
}

PI_USER="${PI_USER:-thong}"
EXPLICIT_HOST_SET=false
PI_HOST=""

# Parse CLI arguments
DO_BUILD=true
FORCE_BUILD=false
FOLLOW_LOGS=false
TARGET_SERVICE=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --help|-h)
            echo "Cách dùng: $0 [--no-build] [--force-build|-f] [--logs|-l] [service_name] [custom_host_or_ip]"
            echo "service_name: edge-router | edge-agent | edge-network | edge-camera | edge-vision | edge-vision-v1"
            echo "              (alias cũ vẫn nhận: mavlink, cc-agent, networking, camera, vision, vision-v1, ...)"
            echo ""
            echo "Tùy chọn tối ưu hóa:"
            echo "  --no-build            Bỏ qua build, chỉ đồng bộ file (rsync) và rolling-restart"
            echo "  --force-build, -f     Bắt buộc build lại toàn bộ image bỏ qua cache dirty-check"
            echo "  --logs, -l            Tự động theo dõi logs sau khi deploy"
            echo ""
            echo "Ví dụ:"
            echo "  $0                    # Deploy toàn bộ stack (tự động phân giải mDNS, smart dirty build)"
            echo "  $0 edge-agent         # Deploy siêu tốc riêng edge-agent (không khởi động lại container khác)"
            echo "  $0 --no-build         # Deploy nhanh nhất (< 2s) khi chỉ đổi config hoặc đã build xong"
            echo "  $0 10.14.95.6         # Deploy chỉ định IP cụ thể"
            exit 0
            ;;
        --no-build)
            DO_BUILD=false
            shift
            ;;
        --force-build|-f)
            FORCE_BUILD=true
            shift
            ;;
        --logs|-l)
            FOLLOW_LOGS=true
            shift
            ;;
        hardware-manager|hw-manager|hw)
            echo -e "${YELLOW}ℹ Lưu ý: hardware-manager đã được gộp trực tiếp vào edge-agent. Đang chuyển target sang edge-agent...${NC}"
            TARGET_SERVICE="edge-agent"
            shift
            ;;
        edge-router|router|mavlink|mavlink-router-controller|drone-mavlink)
            TARGET_SERVICE="edge-router"
            shift
            ;;
        edge-agent|agent|cc-agent|cc)
            TARGET_SERVICE="edge-agent"
            shift
            ;;
        edge-network|network|networking|wifi-ap|drone-networking)
            TARGET_SERVICE="edge-network"
            shift
            ;;
        edge-camera|camera|camera-stream-controller|camera-rtsp|drone-camera-rtsp)
            TARGET_SERVICE="edge-camera"
            shift
            ;;
        edge-vision|vision|drone-vision)
            TARGET_SERVICE="edge-vision"
            shift
            ;;
        edge-vision-v1|vision-v1|vision_v1)
            TARGET_SERVICE="edge-vision-v1"
            shift
            ;;
        *@*)
            PI_USER="${1%@*}"
            PI_HOST="$(normalize_pi_host "${1#*@}")"
            EXPLICIT_HOST_SET=true
            shift
            ;;
        *)
            PI_HOST="$(normalize_pi_host "$1")"
            EXPLICIT_HOST_SET=true
            shift
            ;;
    esac
done

if [ "$EXPLICIT_HOST_SET" = false ]; then
    RESOLVED_HOST="$(bash "${WORKSPACE_DIR}/deploy/ship/resolve_target.sh")"
    PI_HOST="$(echo "$RESOLVED_HOST" | tail -n 1)"
fi

sync_ssh_config() {
    local target_host="$1"
    local target_user="$2"
    local ssh_cfg="$HOME/.ssh/config"

    mkdir -p "$HOME/.ssh"
    touch "$ssh_cfg"
    chmod 600 "$ssh_cfg"
    sed -i '/# --- DRONE-PI-CONFIG-START ---/,/# --- DRONE-PI-CONFIG-END ---/d' "$ssh_cfg"

    cat >> "$ssh_cfg" <<EOF
# --- DRONE-PI-CONFIG-START ---
Host drone-pi
    HostName ${target_host}
    User ${target_user}
    IdentityFile ~/.ssh/id_ed25519
    StrictHostKeyChecking accept-new
    LogLevel ERROR
# --- DRONE-PI-CONFIG-END ---
EOF
}

echo -e "${BLUE}======================================================${NC}"
echo -e "${CYAN}   🚀 DRONE EDGE - FAST ROLLING DEPLOY${NC}"
echo -e "   Target:       ${GREEN}${PI_USER}@${PI_HOST}${NC}"
echo -e "   Service:      ${YELLOW}${TARGET_SERVICE:-ALL (Smart Rolling Update)}${NC}"
echo -e "${BLUE}======================================================${NC}"

# 1. Kiểm tra kết nối SSH tới Pi
echo -e "\n${YELLOW}[1/4] Kiểm tra kết nối SSH tới ${PI_USER}@${PI_HOST}...${NC}"
sync_ssh_config "$PI_HOST" "$PI_USER"

check_ssh() {
    ssh -o BatchMode=yes -o ConnectTimeout=4 -o StrictHostKeyChecking=accept-new \
        "${PI_USER}@${PI_HOST}" "echo 'SSH_OK'" >/dev/null 2>&1
}

if ! check_ssh; then
    echo -e "${RED}✗ Không kết nối được tới ${PI_USER}@${PI_HOST}!${NC}"
    echo -e "${YELLOW}Vui lòng kiểm tra địa chỉ IP trong target.env hoặc đảm bảo Pi đã bật nguồn.${NC}"
    exit 1
else
    echo -e "${GREEN}✓ Kết nối SSH thành công!${NC}"
fi

# Service (= container = image) -> module directory under src/ (docker build context is the repo root)
service_dir() {
    case "$1" in
        edge-router)  echo "src/modules/mavlink_router" ;;
        edge-agent)   echo "platforms/linux/edge_agent" ;;
        edge-network) echo "src/modules/networking" ;;
        edge-camera)  echo "src/modules/camera_streamer" ;;
        edge-vision)  echo "src/modules/vision" ;;
        edge-vision-v1) echo "src/modules/vision_v1" ;;
        *) echo "" ;;
    esac
}

# Shared code every image may pull in: a change here dirties every service.
SHARED_PATHS=("src/lib" "src/drivers" "msg" "cmake" "platforms/linux/docker")

service_fingerprint() {
    local rel
    rel="$(service_dir "$1")"
    ( cd "$WORKSPACE_DIR" && git status --porcelain "$rel" "${SHARED_PATHS[@]}" 2>/dev/null; \
      git log -1 --format="%H" -- "$rel" "${SHARED_PATHS[@]}" 2>/dev/null )
}

# 2. Smart Build ARM64 Docker Images (Smart Dirty Check)
is_service_dirty() {
    local svc="$1"
    local img_name="$2"
    
    # Chưa có image local -> bắt buộc build
    if ! docker image inspect "$img_name" >/dev/null 2>&1; then
        return 0
    fi

    if [ "$FORCE_BUILD" = true ]; then
        return 0
    fi

    # Kiểm tra dirty fingerprint qua git
    local cache_file="${CACHE_DIR}/${svc}.sum"
    local current_sum
    current_sum=$(service_fingerprint "$svc")

    if [ -f "$cache_file" ] && [ "$(cat "$cache_file")" = "$current_sum" ]; then
        return 1 # CLEAN
    fi
    
    return 0 # DIRTY
}

mark_service_clean() {
    local svc="$1"
    local cache_file="${CACHE_DIR}/${svc}.sum"
    local current_sum
    current_sum=$(service_fingerprint "$svc")
    echo "$current_sum" > "$cache_file"
}

# Service -> target in docker-bake.hcl. Every image is FROM edge-ros-base/edge-ros-builder, which bake resolves
# as named contexts (a plain `docker buildx build --file` cannot see those local images).
bake_target() {
    case "$1" in
        edge-router)  echo "router" ;;
        edge-agent)   echo "agent" ;;
        edge-network) echo "network" ;;
        edge-camera)  echo "camera" ;;
        edge-vision)  echo "vision" ;;
        edge-vision-v1) echo "vision-v1" ;;
        *) echo "" ;;
    esac
}

build_single_service() {
    local svc="$1"
    local img_tag="$2"
    local module_dir
    module_dir="$(service_dir "$svc")"
    local dockerfile="${WORKSPACE_DIR}/${module_dir}/Dockerfile"

    if [ ! -f "$dockerfile" ]; then
        echo -e "${RED}✗ Không tìm thấy Dockerfile tại ${dockerfile}!${NC}"
        return 1
    fi

    if is_service_dirty "$svc" "$img_tag"; then
        echo -e "     ▶ ${CYAN}Đang build chéo ARM64 cho ${svc} (${img_tag})...${NC}"
        ( cd "$WORKSPACE_DIR" && docker buildx bake -f docker-bake.hcl --builder drone-builder \
            --set "*.platform=linux/arm64" --load "$(bake_target "$svc")" )
        mark_service_clean "$svc"
        echo -e "       ${GREEN}✓ Build ${svc} hoàn tất!${NC}"
    else
        echo -e "     ⚡ ${GREEN}Mã nguồn ${svc} không đổi, tái sử dụng image có sẵn.${NC}"
    fi
}

if [ "$DO_BUILD" = true ]; then
    echo -e "\n${YELLOW}[2/4] Kiểm tra và Build ARM64 Docker images (Smart Cache)...${NC}"
    # Tự động khắc phục lỗi stale bind mount của Docker Desktop trên WSL2
    if ! docker buildx inspect drone-builder >/dev/null 2>&1; then
        docker buildx create --name drone-builder --driver docker-container --bootstrap
    elif ! docker buildx inspect drone-builder --bootstrap >/dev/null 2>&1; then
        echo -e "     ${YELLOW}⚡ Phát hiện builder cũ bị stale do WSL/Docker restart. Đang tái tạo drone-builder...${NC}"
        docker buildx rm -f drone-builder >/dev/null 2>&1 || true
        docker buildx create --name drone-builder --driver docker-container --bootstrap
    fi

    if [ -n "$TARGET_SERVICE" ]; then
        build_single_service "$TARGET_SERVICE" "${TARGET_SERVICE}:latest"
    else
        build_single_service "edge-router" "edge-router:latest"
        build_single_service "edge-agent" "edge-agent:latest"
        build_single_service "edge-network" "edge-network:latest"
        build_single_service "edge-camera" "edge-camera:latest"
    fi
else
    echo -e "\n${YELLOW}[2/4] Bỏ qua build (--no-build), sử dụng image hiện có.${NC}"
fi

# 3. Đồng bộ Docker image sang Raspberry Pi (Local Registry qua SSH Reverse Tunnel, fallback Delta Pipe)
echo -e "\n${YELLOW}[3/4] Đồng bộ Image sang Raspberry Pi...${NC}"

# Nhận diện compressor tối ưu nếu cần fallback
COMPRESSOR="gzip -1"
DECOMPRESSOR="gunzip -c"
if command -v pigz >/dev/null 2>&1; then
    COMPRESSOR="pigz -1 -p 4"
fi
if ssh -o BatchMode=yes -o ConnectTimeout=3 "${PI_USER}@${PI_HOST}" "command -v pigz" >/dev/null 2>&1; then
    DECOMPRESSOR="pigz -dc"
fi

IMAGES_TO_SYNC=()
if [ -n "$TARGET_SERVICE" ]; then
    IMAGES_TO_SYNC=("$TARGET_SERVICE:latest")
else
    for img in "edge-router:latest" "edge-agent:latest" "edge-network:latest" "edge-camera:latest"; do
        if docker image inspect "$img" >/dev/null 2>&1; then
            IMAGES_TO_SYNC+=("$img")
        fi
    done
fi

IMAGES_TO_SEND=()
for img in "${IMAGES_TO_SYNC[@]}"; do
    local_id=$(docker image inspect "${img}" --format '{{.Id}}' 2>/dev/null || true)
    if [ -z "$local_id" ]; then continue; fi
    remote_id=$(ssh "${PI_USER}@${PI_HOST}" "docker image inspect '${img}' --format '{{.Id}}' 2>/dev/null || true")

    if [ "${local_id}" = "${remote_id}" ]; then
        echo -e "     ⚡ ${GREEN}${img} đã tồn tại với cùng Image ID trên Pi; bỏ qua truyền.${NC}"
        continue
    fi
    IMAGES_TO_SEND+=("$img")
done

if [ "${#IMAGES_TO_SEND[@]}" -gt 0 ]; then
    # Kiểm tra và khởi động Local Docker Registry nếu chưa có
    REGISTRY_READY=false
    if curl -s -f http://localhost:5000/v2/ >/dev/null 2>&1; then
        REGISTRY_READY=true
    else
        if docker start cc-registry >/dev/null 2>&1 || docker run -d -p 5000:5000 --name cc-registry --restart=always registry:2 >/dev/null 2>&1; then
            sleep 1
            if curl -s -f http://localhost:5000/v2/ >/dev/null 2>&1; then
                REGISTRY_READY=true
            fi
        fi
    fi

    if [ "$REGISTRY_READY" = true ]; then
        echo -e "     🚀 ${CYAN}Đồng bộ siêu tốc qua Local Registry (SSH Reverse Tunnel)...${NC}"
        for img in "${IMAGES_TO_SEND[@]}"; do
            echo -e "        ▶ Đang đẩy ${img} lên registry local..."
            docker tag "$img" "localhost:5000/$img"
            docker push "localhost:5000/$img" >/dev/null 2>&1
        done

        ssh -R 5000:localhost:5000 "${PI_USER}@${PI_HOST}" bash -s <<REMOTE_EOF
            for img in ${IMAGES_TO_SEND[*]}; do
                echo "        ▶ Kéo \$img từ registry local..."
                docker pull "localhost:5000/\$img" >/dev/null 2>&1
                docker tag "localhost:5000/\$img" "\$img"
                docker rmi "localhost:5000/\$img" >/dev/null 2>&1 || true
            done
REMOTE_EOF
        echo -e "       ${GREEN}✓ Nạp ${#IMAGES_TO_SEND[@]} image qua Registry thành công!${NC}"
    else
        echo -e "     ▶ Đang stream ${CYAN}${IMAGES_TO_SEND[*]}${NC} sang Pi (${COMPRESSOR} ➔ ${DECOMPRESSOR})..."
        docker save "${IMAGES_TO_SEND[@]}" | $COMPRESSOR | \
            ssh "${PI_USER}@${PI_HOST}" "$DECOMPRESSOR | docker load"
        echo -e "       ${GREEN}✓ Nạp ${#IMAGES_TO_SEND[@]} image thành công!${NC}"
    fi
fi

# 4. Đồng bộ cấu hình & Rolling update containers trên Pi
echo -e "\n${YELLOW}[4/4] Đồng bộ cấu hình bằng rsync & Rolling Update Container...${NC}"

# config/ is the drone's own persistent state: seeded once, never overwritten by deploys.
seed_remote_config() {
    ssh "${PI_USER}@${PI_HOST}" "mkdir -p ${REMOTE_DIR}/config && cd ${REMOTE_DIR} && \
        { [ -f config/wifi.json ] || [ ! -f src/modules/networking/wifi.json ] || cp src/modules/networking/wifi.json config/wifi.json; }"
    if ! ssh "${PI_USER}@${PI_HOST}" "test -f ${REMOTE_DIR}/config/wifi.json"; then
        local src="${WORKSPACE_DIR}/config/wifi.json"
        if [ ! -f "$src" ]; then
            src="${WORKSPACE_DIR}/config/wifi.example.json"
            echo -e "${YELLOW}⚠ Không có config/wifi.json ở local — đang seed mật khẩu hotspot mẫu (public); hãy đổi từ QGC và Save as default.${NC}"
        fi
        rsync -a "$src" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/config/wifi.json"
    fi
}

# links.json: seeded once and never overwritten. First deploy migrates the port/baud pairs that older
# deployments kept in the Pi's .env, so field-tuned values survive the move to config/links.json.
seed_remote_links() {
    ssh "${PI_USER}@${PI_HOST}" "cd ${REMOTE_DIR} && mkdir -p config && [ -f config/links.json ] || python3 - <<'PY'
import json, os, re
env = dict(re.findall(r'^([A-Z_]+)=(.*)\$', open('.env').read(), re.M)) if os.path.exists('.env') else {}
links = [{'name': n, 'port': env[p], 'baud': int(env.get(b) or d)}
         for n, p, b, d in (('FC', 'DRONE_SERIAL_PORT', 'DRONE_BAUD_RATE', 921600),
                            ('SIYI', 'SIYI_SERIAL_PORT', 'SIYI_BAUD', 115200)) if env.get(p)]
if links:
    json.dump({'links': links}, open('config/links.json', 'w'), indent=2)
PY"
    ssh "${PI_USER}@${PI_HOST}" "test -f ${REMOTE_DIR}/config/links.json" || \
        rsync -a "${WORKSPACE_DIR}/config/links.example.json" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/config/links.json"
}

# Must run before `docker compose up`: otherwise Docker creates config/ as root and later seeding fails.
seed_remote_config
seed_remote_links

if ssh "${PI_USER}@${PI_HOST}" "grep -q 'change-this-hotspot-password' ${REMOTE_DIR}/config/wifi.json" 2>/dev/null; then
    echo -e "${YELLOW}⚠ Hotspot vẫn dùng mật khẩu mẫu — đổi từ QGC (Network → AP password → Save as default).${NC}"
fi

ssh "${PI_USER}@${PI_HOST}" "mkdir -p ${REMOTE_DIR}/src ${REMOTE_DIR}/deploy ${REMOTE_DIR}/boards"

RSYNC_EXCLUDES=(
    --exclude='.git*'
    --exclude='build*'
    --exclude='install*'
    --exclude='log*'
    --exclude='__pycache__*'
    --exclude='*.o'
    --exclude='*.a'
    --exclude='*.so'
)

if [ -n "$TARGET_SERVICE" ]; then
    echo -e "     ▶ Đang rsync cấu hình cho ${CYAN}${TARGET_SERVICE}${NC}..."
    TARGET_DIR="$(service_dir "$TARGET_SERVICE")"
    ssh "${PI_USER}@${PI_HOST}" "mkdir -p ${REMOTE_DIR}/${TARGET_DIR}"
    rsync -avz --delete "${RSYNC_EXCLUDES[@]}" \
        "${WORKSPACE_DIR}/${TARGET_DIR}/" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/${TARGET_DIR}/"
    if [ "$TARGET_SERVICE" = "edge-camera" ] || [ "$TARGET_SERVICE" = "edge-vision-v1" ]; then
        ssh "${PI_USER}@${PI_HOST}" "mkdir -p ${REMOTE_DIR}/src/drivers/camera/config"
        rsync -avz --delete "${WORKSPACE_DIR}/src/drivers/camera/config/" \
            "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/src/drivers/camera/config/"
    fi
    if [ "$TARGET_SERVICE" = "edge-vision-v1" ]; then
        # vision_v1 deliberately reuses the existing model asset instead of duplicating it.
        # A targeted deploy therefore has to ship that bind-mounted runtime asset as well.
        ssh "${PI_USER}@${PI_HOST}" "mkdir -p ${REMOTE_DIR}/src/modules/vision"
        rsync -avz "${WORKSPACE_DIR}/src/modules/vision/yolo26n.pt" \
            "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/src/modules/vision/yolo26n.pt"
    fi
    rsync -avz "${WORKSPACE_DIR}/docker-compose.yml" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/"
    [ -f "${WORKSPACE_DIR}/.env" ] && rsync -avz "${WORKSPACE_DIR}/.env" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/"
    rsync -avz "${WORKSPACE_DIR}/deploy/ship/verify-deployment.sh" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/deploy/ship/"

    echo -e "     ▶ Rolling restart container ${CYAN}${TARGET_SERVICE}${NC}..."
    CONFLICTING_VISION_SERVICE=""
    if [ "$TARGET_SERVICE" = "edge-vision-v1" ]; then
        CONFLICTING_VISION_SERVICE="edge-vision"
    elif [ "$TARGET_SERVICE" = "edge-vision" ]; then
        CONFLICTING_VISION_SERVICE="edge-vision-v1"
    fi
    ssh "${PI_USER}@${PI_HOST}" "
        cd ${REMOTE_DIR}
        if [ -n '${CONFLICTING_VISION_SERVICE}' ]; then
            docker compose --profile vision --profile vision-v1 stop '${CONFLICTING_VISION_SERVICE}' || exit \$?
        fi
        docker compose up -d --no-deps --no-build ${TARGET_SERVICE}
        bash deploy/ship/verify-deployment.sh ${TARGET_SERVICE}
    "
    # Thu hồi ảnh rác
    ssh "${PI_USER}@${PI_HOST}" "docker image prune -f >/dev/null 2>&1 || true"
    echo -e "\n${GREEN}======================================================${NC}"
    echo -e "${GREEN}   🎉 DỊCH VỤ ${TARGET_SERVICE} ĐÃ TRIỂN KHAI THÀNH CÔNG!${NC}"
    echo -e "${GREEN}======================================================${NC}"
    exit 0
fi

# Full stack rsync
echo -e "     ▶ Đồng bộ cây thư mục src/ và deploy/..."
rsync -avz --delete "${RSYNC_EXCLUDES[@]}" \
    "${WORKSPACE_DIR}/src/" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/src/"
rsync -avz --delete --exclude='.git*' \
    "${WORKSPACE_DIR}/deploy/" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/deploy/"
rsync -avz --delete --exclude='.git*' \
    "${WORKSPACE_DIR}/boards/" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/boards/"
rsync -avz "${WORKSPACE_DIR}/docker-compose.yml" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/"
[ -f "${WORKSPACE_DIR}/.env" ] && rsync -avz "${WORKSPACE_DIR}/.env" "${PI_USER}@${PI_HOST}:${REMOTE_DIR}/"

echo -e "     ▶ In-place Rolling Update toàn bộ dịch vụ..."
ssh "${PI_USER}@${PI_HOST}" "
    cd ${REMOTE_DIR}
    # Rolling update: chỉ recreate các container có image/cấu hình thay đổi, KHÔNG làm rớt WiFi AP
    docker compose up -d --remove-orphans --no-build
    echo ''
    docker compose ps
"

# 5. Tự động dọn dẹp Docker rác (Auto Clean Sweep)
echo -e "\n${YELLOW}[5/5] Tự động dọn dẹp Docker rác và giải phóng bộ nhớ trên Pi...${NC}"
ssh "${PI_USER}@${PI_HOST}" "
    docker image prune -f >/dev/null 2>&1 || true
    docker container prune -f >/dev/null 2>&1 || true
"
echo -e "     ${GREEN}✓ Thu hồi bộ nhớ và dọn sạch image rác thành công!${NC}"

echo -e "\n${GREEN}======================================================${NC}"
echo -e "${GREEN}   🎉 TRIỂN KHAI HOÀN TẤT TRÊN RASPBERRY PI!${NC}"
echo -e "${GREEN}======================================================${NC}"
echo -e "Các đường link truy cập:"
echo -e "   - WiFi Hotspot:               SSID: AP_DRONE"
echo -e "   - MAVLink GCS UDP:            Any reachable Pi IP:14550 (no broadcast)"
echo -e "   - MAVLink GCS SIYI:           UDP 14550 (192.168.144.20)"
echo -e "   - MAVLink TCP:                ${PI_HOST}:5760"
echo -e "   - RTSP camera:                ${CYAN}rtsp://${PI_HOST}:8554/camera${NC}"
echo -e "   - RTSP YOLO (vision profile): ${CYAN}rtsp://${PI_HOST}:8554/yolo${NC}"
echo -e "   - WebRTC:                     ${CYAN}http://${PI_HOST}:8889/camera${NC}"
echo -e "   - AP recovery (no mDNS):      192.168.10.1"

if [ "$FOLLOW_LOGS" = true ]; then
    echo -e "\n${BLUE}▶ Đang mở log realtime (Ctrl+C để thoát)...${NC}"
    ssh -t "${PI_USER}@${PI_HOST}" "docker compose -f ${REMOTE_DIR}/docker-compose.yml logs -f --tail=50"
fi
