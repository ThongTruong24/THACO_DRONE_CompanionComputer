#!/usr/bin/env bash
# ==============================================================================
# deploy/setup/setup.sh - Unified Provisioning & Hardware Setup Suite
# ==============================================================================
# Gộp toàn bộ quy trình thiết lập môi trường cho Drone Edge:
#   1. Cấu hình SSH Key Authentication không cần mật khẩu
#   2. Cài đặt và cấu hình Docker Runtime
#   3. Thiết lập Hardware Abstraction Layer (HAL): UART overlays, Encoder, .env
#
# Cách dùng:
#   sudo ./deploy/setup/setup.sh                  # Chạy trực tiếp trên bo mạch
#   ./deploy/setup/setup.sh --host 10.14.95.6     # Chạy từ xa từ máy dev
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DRONE_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
HARDWARE_DIR="${DRONE_ROOT}/deploy/setup/hardware"
ENV_FILE="${DRONE_ROOT}/.env"

# ANSI Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

# Default Options
REMOTE_HOST=""
REMOTE_USER="thong"
DO_SSH=""
DO_DOCKER=""
DO_HARDWARE=""
PROFILE=""
AUTO_YES=false
DRY_RUN=false

usage() {
    echo -e "${BOLD}Sử dụng:${NC} $0 [OPTIONS]"
    echo ""
    echo "Tùy chọn chung:"
    echo "  --host <ip/name>        Chạy thiết lập từ xa tới bo mạch đích (qua SSH)"
    echo "  --user <name>           Tên user trên bo mạch đích (mặc định: thong)"
    echo "  --profile, -p <name>    Chỉ định profile phần cứng: rpi5 | rpi4 | jetson"
    echo "  -y, --yes               Tự động đồng ý tất cả các câu hỏi (Non-interactive)"
    echo "  --dry-run               Hiển thị các thao tác mà không ghi đè file"
    echo ""
    echo "Chạy đơn lẻ từng phần:"
    echo "  --ssh                   Chỉ chạy cấu hình SSH Key"
    echo "  --docker                Chỉ chạy cài đặt Docker"
    echo "  --hardware              Chỉ chạy cấu hình phần cứng"
    echo "  --help, -h              Hiển thị trợ giúp này"
    echo ""
    exit 0
}

# Parse CLI arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --host)
            REMOTE_HOST="$2"
            shift 2
            ;;
        --user)
            REMOTE_USER="$2"
            shift 2
            ;;
        --profile|-p)
            PROFILE="$2"
            shift 2
            ;;
        -y|--yes)
            AUTO_YES=true
            shift
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --ssh)
            DO_SSH="yes"
            shift
            ;;
        --docker)
            DO_DOCKER="yes"
            shift
            ;;
        --hardware)
            DO_HARDWARE="yes"
            shift
            ;;
        --help|-h)
            usage
            ;;
        *)
            echo -e "${RED}[Lỗi] Tham số không hợp lệ: $1${NC}"
            usage
            ;;
    esac
done

ask_confirm() {
    local prompt="$1"
    local default_yes="${2:-true}"
    if [ "$AUTO_YES" = true ]; then
        return 0
    fi
    local answer
    if [ "$default_yes" = true ]; then
        read -r -p "$prompt [Y/n]: " answer
        answer="${answer:-Y}"
        [[ "$answer" =~ ^[Yy]$ ]] && return 0 || return 1
    else
        read -r -p "$prompt [y/N]: " answer
        answer="${answer:-N}"
        [[ "$answer" =~ ^[Yy]$ ]] && return 0 || return 1
    fi
}

echo -e "\n${CYAN}======================================================================${NC}"
echo -e "${CYAN}    🛸 THACO AGRI DRONE — BỘ THIẾT LẬP HỆ THỐNG & PHẦN CỨNG           ${NC}"
echo -e "${CYAN}======================================================================${NC}"

# ==============================================================================
# CHẾ ĐỘ 1: CHẠY TỪ XA TỚI BO MẠCH (REMOTE PROVISIONING)
# ==============================================================================
if [ -n "$REMOTE_HOST" ]; then
    echo -e "${BLUE}▶ Đích đến từ xa: ${GREEN}${REMOTE_USER}@${REMOTE_HOST}${NC}"

    # 1. Cấu hình SSH Key
    if [ -z "$DO_SSH" ] && [ -z "$DO_DOCKER" ] && [ -z "$DO_HARDWARE" ]; then
        if ask_confirm "Bạn có muốn thiết lập kết nối SSH Key không cần mật khẩu?"; then
            DO_SSH="yes"
        fi
    fi

    if [ "$DO_SSH" = "yes" ]; then
        echo -e "\n${YELLOW}--- [1/3] Cấu hình SSH Key Authentication ---${NC}"
        SSH_KEY="$HOME/.ssh/id_ed25519"
        mkdir -p "$HOME/.ssh" && chmod 700 "$HOME/.ssh"
        if [ ! -f "$SSH_KEY" ]; then
            echo "  Tạo SSH Key ed25519 mới..."
            ssh-keygen -t ed25519 -N "" -f "$SSH_KEY" -C "drone-dev"
        fi
        echo "  Sao chép public key sang ${REMOTE_HOST}..."
        ssh-copy-id -i "${SSH_KEY}.pub" -o StrictHostKeyChecking=no "${REMOTE_USER}@${REMOTE_HOST}" || true
        echo -e "${GREEN}✓ Đã cấu hình SSH Key!${NC}"
    fi

    # 2. Cài đặt Docker từ xa
    if [ -z "$DO_DOCKER" ] && [ -z "$DO_SSH" ] && [ -z "$DO_HARDWARE" ]; then
        if ask_confirm "Bạn có muốn kiểm tra / cài đặt Docker trên bo mạch?"; then
            DO_DOCKER="yes"
        fi
    fi

    if [ "$DO_DOCKER" = "yes" ]; then
        echo -e "\n${YELLOW}--- [2/3] Cài đặt Docker trên ${REMOTE_HOST} ---${NC}"
        ssh -t "${REMOTE_USER}@${REMOTE_HOST}" "
            if command -v docker >/dev/null 2>&1; then
                echo 'Docker đã được cài đặt:'; docker --version
            else
                echo 'Đang cài đặt Docker...'
                curl -fsSL https://get.docker.com | sudo sh
                sudo usermod -aG docker \$USER
                echo 'Docker cài đặt thành công!'
            fi
        "
        echo -e "${GREEN}✓ Hoàn tất cấu hình Docker!${NC}"
    fi

    # 3. Nạp Hardware Profile từ xa
    if [ -z "$DO_HARDWARE" ] && [ -z "$DO_SSH" ] && [ -z "$DO_DOCKER" ]; then
        if ask_confirm "Bạn có muốn cấu hình phần cứng (UART, Video, .env) trên bo mạch?"; then
            DO_HARDWARE="yes"
        fi
    fi

    if [ "$DO_HARDWARE" = "yes" ]; then
        echo -e "\n${YELLOW}--- [3/3] Nạp cấu hình Phần cứng (HAL) ---${NC}"
        # Đồng bộ thư mục deploy/ lên bo mạch trước
        ssh "${REMOTE_USER}@${REMOTE_HOST}" "mkdir -p ~/drone-edge"
        scp -r "${DRONE_ROOT}/deploy" "${REMOTE_USER}@${REMOTE_HOST}:~/drone-edge/"
        
        FLAG_PROFILE=""
        [ -n "$PROFILE" ] && FLAG_PROFILE="--profile ${PROFILE}"
        ssh -t "${REMOTE_USER}@${REMOTE_HOST}" "cd ~/drone-edge && sudo ./deploy/setup/setup.sh --hardware ${FLAG_PROFILE} -y"
    fi

    echo -e "\n${GREEN}======================================================================${NC}"
    echo -e "${GREEN}   🎉 THIẾT LẬP TỪ XA HOÀN TẤT TRÊN ${REMOTE_HOST}!                 ${NC}"
    echo -e "${GREEN}======================================================================${NC}"
    exit 0
fi

# ==============================================================================
# CHẾ ĐỘ 2: CHẠY TRỰC TIẾP TRÊN BO MẠCH (LOCAL PROVISIONING)
# ==============================================================================

# Nếu không chỉ định cờ cụ thể nào, hỏi người dùng theo dạng wizard tương tác
if [ -z "$DO_SSH" ] && [ -z "$DO_DOCKER" ] && [ -z "$DO_HARDWARE" ]; then
    echo -e "\n${BOLD}Vui lòng chọn các hạng mục cần thiết lập:${NC}"
    ask_confirm "1. Bạn có muốn kiểm tra / cài đặt Docker không?" true && DO_DOCKER="yes" || DO_DOCKER="no"
    ask_confirm "2. Bạn có muốn cấu hình phần cứng (UART overlays, video encoder, .env) không?" true && DO_HARDWARE="yes" || DO_HARDWARE="no"
fi

# 1. Cài đặt Docker tại chỗ
if [ "$DO_DOCKER" = "yes" ]; then
    echo -e "\n${YELLOW}▶ [Docker] Kiểm tra môi trường Docker...${NC}"
    if command -v docker >/dev/null 2>&1; then
        echo -e "  ${GREEN}✓ Docker đã có sẵn:${NC} $(docker --version)"
    else
        echo -e "  ${YELLOW}Docker chưa có. Bắt đầu tải và cài đặt chính thức...${NC}"
        if [ "$DRY_RUN" = false ]; then
            curl -fsSL https://get.docker.com | sudo sh
            TARGET_USER="${SUDO_USER:-$USER}"
            sudo usermod -aG docker "$TARGET_USER" 2>/dev/null || true
            echo -e "  ${GREEN}✓ Docker đã cài đặt thành công! User '$TARGET_USER' đã được cấp quyền docker.${NC}"
        else
            echo "  [Dry-run] Sẽ chạy 'curl -fsSL https://get.docker.com | sudo sh'"
        fi
    fi
fi

# 2. Cấu hình Phần cứng (HAL)
if [ "$DO_HARDWARE" = "yes" ]; then
    echo -e "\n${YELLOW}▶ [Hardware] Cấu hình tầng trừu tượng phần cứng (HAL)...${NC}"
    
    # Phát hiện bo mạch
    DETECTED_MODEL="Unknown"
    if [ -f /proc/device-tree/model ]; then
        DETECTED_MODEL="$(tr -d '\0' < /proc/device-tree/model)"
    elif [ -f /sys/firmware/devicetree/base/model ]; then
        DETECTED_MODEL="$(tr -d '\0' < /sys/firmware/devicetree/base/model)"
    fi
    echo -e "  Bo mạch nhận diện: ${BOLD}${DETECTED_MODEL}${NC}"

    if [ -z "$PROFILE" ]; then
        if echo "$DETECTED_MODEL" | grep -qi "Raspberry Pi 5"; then
            PROFILE="rpi5"
        elif echo "$DETECTED_MODEL" | grep -qi "Raspberry Pi 4"; then
            PROFILE="rpi4"
        elif echo "$DETECTED_MODEL" | grep -qi -E "Jetson|NVIDIA"; then
            PROFILE="jetson"
        else
            PROFILE="rpi5"
            echo -e "  ${YELLOW}[Lưu ý] Không tự động nhận diện được, mặc định dùng: ${PROFILE}${NC}"
        fi
    fi

    PROFILE_DIR="${HARDWARE_DIR}/${PROFILE}"
    if [ ! -d "$PROFILE_DIR" ]; then
        echo -e "${RED}[Lỗi] Không tìm thấy thư mục profile: ${PROFILE_DIR}${NC}" >&2
        exit 1
    fi
    echo -e "  Áp dụng Profile: ${GREEN}${BOLD}${PROFILE}${NC}"

    # Cập nhật .env
    PROFILE_ENV="${PROFILE_DIR}/hardware.env"
    if [ -f "$PROFILE_ENV" ]; then
        echo "  Cập nhật file .env từ ${PROFILE}/hardware.env..."
        if [ "$DRY_RUN" = false ]; then
            touch "$ENV_FILE"
            while IFS='=' read -r key val || [ -n "$key" ]; do
                [[ "$key" =~ ^#.*$ ]] && continue
                [[ -z "$key" ]] && continue
                if grep -q "^${key}=" "$ENV_FILE"; then
                    sed -i "s|^${key}=.*|${key}=${val}|" "$ENV_FILE"
                else
                    echo "${key}=${val}" >> "$ENV_FILE"
                fi
            done < "$PROFILE_ENV"
            echo -e "  ${GREEN}✓ File .env đã cập nhật!${NC}"
        else
            echo "  [Dry-run] Sẽ trộn nội dung từ: $PROFILE_ENV"
        fi
    fi

    # Cập nhật boot firmware (/boot/firmware/config.txt)
    NEED_REBOOT=false
    if [[ "$PROFILE" =~ ^rpi[0-9]+ ]]; then
        BOOT_SRC="${PROFILE_DIR}/config.txt"
        BOOT_DEST=""
        [ -f /boot/firmware/config.txt ] && BOOT_DEST="/boot/firmware/config.txt"
        [ -z "$BOOT_DEST" ] && [ -f /boot/config.txt ] && BOOT_DEST="/boot/config.txt"

        if [ -n "$BOOT_DEST" ] && [ -f "$BOOT_SRC" ]; then
            echo "  Kiểm tra Boot Firmware Overlay (${BOOT_DEST})..."
            if ! cmp -s "$BOOT_SRC" "$BOOT_DEST"; then
                if [ "$DRY_RUN" = false ]; then
                    if [[ $EUID -ne 0 ]]; then
                        echo -e "  ${YELLOW}Yêu cầu sudo để ghi vào ${BOOT_DEST}...${NC}"
                        sudo cp "$BOOT_DEST" "${BOOT_DEST}.bak.$(date +%Y%m%d_%H%M%S)" || true
                        sudo cp "$BOOT_SRC" "$BOOT_DEST"
                    else
                        cp "$BOOT_DEST" "${BOOT_DEST}.bak.$(date +%Y%m%d_%H%M%S)" || true
                        cp "$BOOT_SRC" "$BOOT_DEST"
                    fi
                    echo -e "  ${GREEN}✓ Boot firmware overlay đã cập nhật thành công!${NC}"
                    NEED_REBOOT=true
                else
                    echo "  [Dry-run] Sẽ sao chép $BOOT_SRC -> $BOOT_DEST"
                fi
            else
                echo -e "  ${GREEN}✓ Boot config.txt đã đúng chuẩn, không cần ghi đè.${NC}"
            fi
        fi
    fi

    # Thêm user vào group dialout
    TARGET_USER="${SUDO_USER:-$USER}"
    if [ "$TARGET_USER" != "root" ]; then
        if [ "$DRY_RUN" = false ]; then
            sudo usermod -aG dialout "$TARGET_USER" 2>/dev/null || true
            echo -e "  ${GREEN}✓ User '$TARGET_USER' đã được cấp quyền truy cập UART (dialout).${NC}"
        else
            echo "  [Dry-run] Sẽ cấp quyền dialout cho '$TARGET_USER'"
        fi
    fi

    # Kiểm tra cổng UART
    if [ -e /dev/ttyAMA4 ]; then
        echo -e "  ${GREEN}✓ Cổng /dev/ttyAMA4 đã sẵn sàng!${NC}"
    else
        echo -e "  ${YELLOW}⚠ Cổng /dev/ttyAMA4 chưa xuất hiện (Cần reboot để nạp Device Tree).${NC}"
        NEED_REBOOT=true
    fi

    # Xử lý reboot nếu cần
    if [ "$NEED_REBOOT" = true ] && [ "$DRY_RUN" = false ]; then
        echo -e "\n${YELLOW}======================================================================${NC}"
        echo -e "${YELLOW}  ⚠ YÊU CẦU KHỞI ĐỘNG LẠI ĐỂ ÁP DỤNG THAY ĐỔI CỔNG SERIAL (DEVICE TREE)${NC}"
        echo -e "${YELLOW}======================================================================${NC}"
        if ask_confirm "Bạn có muốn khởi động lại (reboot) ngay bây giờ không?" false; then
            echo "Đang khởi động lại sau 3 giây..."
            sleep 3
            sudo reboot
        else
            echo -e "Vui lòng khởi động lại thủ công sau: ${BOLD}sudo reboot${NC}"
        fi
    fi
fi

echo -e "\n${GREEN}======================================================================${NC}"
echo -e "${GREEN}   🎉 THIẾT LẬP HOÀN TẤT THÀNH CÔNG!                                 ${NC}"
echo -e "${GREEN}======================================================================${NC}"
