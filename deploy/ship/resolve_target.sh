#!/usr/bin/env bash
# ==============================================================================
# deploy/ship/resolve_target.sh
# Phân giải thông minh Target Host: Ưu tiên mDNS (drone.local) -> Fallback IP
# ==============================================================================

set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_ENV="${SCRIPT_DIR}/target.env"
ROOT_ENV="${SCRIPT_DIR}/../../.env"

# 1. Lưu biến môi trường truyền từ bên ngoài (nếu có)
ENV_MDNS="${TARGET_MDNS:-}"
ENV_FALLBACK_IP="${TARGET_FALLBACK_IP:-}"
ENV_USER="${TARGET_USER:-}"

# 2. Đọc cấu hình từ target.env nếu có, fallback về .env hoặc mặc định
if [ -f "$TARGET_ENV" ]; then
    # shellcheck disable=SC1090
    source "$TARGET_ENV"
elif [ -f "$ROOT_ENV" ]; then
    # shellcheck disable=SC1090
    source "$ROOT_ENV"
fi

TARGET_MDNS="${ENV_MDNS:-${TARGET_MDNS:-${MDNS_HOSTNAME:-drone}.local}}"
TARGET_FALLBACK_IP="${ENV_FALLBACK_IP:-${TARGET_FALLBACK_IP:-${PI_HOST:-10.14.95.6}}}"
TARGET_USER="${ENV_USER:-${TARGET_USER:-${PI_USER:-thong}}}"

# Nếu TARGET_FALLBACK_IP lại là .local thì đổi thành IP mặc định
if [[ "$TARGET_FALLBACK_IP" == *".local"* ]]; then
    TARGET_FALLBACK_IP="10.14.95.6"
fi

QUIET=false
EXPLICIT_TARGET=""

for arg in "$@"; do
    case "$arg" in
        --quiet|-q)
            QUIET=true
            ;;
        --help|-h)
            echo "Cách dùng: $0 [--quiet] [custom_host_or_ip]"
            exit 0
            ;;
        *)
            if [ -z "$EXPLICIT_TARGET" ]; then
                EXPLICIT_TARGET="$arg"
            fi
            ;;
    esac
done

# Định nghĩa màu
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

check_ssh_port() {
    local host="$1"
    timeout 1.5 bash -c "</dev/tcp/${host}/22" 2>/dev/null
    return $?
}

# Nếu có tham số host chỉ định trực tiếp từ CLI
if [ -n "$EXPLICIT_TARGET" ]; then
    if [ "$QUIET" = false ]; then
        echo -e "${CYAN}[Target Resolver] Sử dụng Host chỉ định trực tiếp:${NC} ${EXPLICIT_TARGET}" >&2
    fi
    echo "$EXPLICIT_TARGET"
    exit 0
fi

# Bước 1: Kiểm tra mDNS (TARGET_MDNS)
if check_ssh_port "$TARGET_MDNS"; then
    if [ "$QUIET" = false ]; then
        echo -e "${GREEN}[Target Resolver] ✓ mDNS '${TARGET_MDNS}' đang hoạt động!${NC}" >&2
    fi
    echo "$TARGET_MDNS"
    exit 0
fi

# Bước 2: Fallback về IP cố định (TARGET_FALLBACK_IP)
if [ -n "$TARGET_FALLBACK_IP" ] && check_ssh_port "$TARGET_FALLBACK_IP"; then
    if [ "$QUIET" = false ]; then
        echo -e "${YELLOW}[Target Resolver] ⚠️  mDNS '${TARGET_MDNS}' không phản hồi. Fallback về IP: ${TARGET_FALLBACK_IP}${NC}" >&2
    fi
    echo "$TARGET_FALLBACK_IP"
    exit 0
fi

# Nếu cả hai đều không thông SSH port 22
if [ "$QUIET" = false ]; then
    echo -e "${RED}[Target Resolver] ✗ Cảnh báo: Cả mDNS '${TARGET_MDNS}' và Fallback IP '${TARGET_FALLBACK_IP}' đều không phản hồi port 22!${NC}" >&2
    echo -e "${YELLOW}[Target Resolver] Mặc định vẫn trả về '${TARGET_MDNS}' để thử kết nối.${NC}" >&2
fi
echo "$TARGET_MDNS"
exit 0
