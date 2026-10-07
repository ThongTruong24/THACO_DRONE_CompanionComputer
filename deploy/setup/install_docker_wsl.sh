#!/bin/bash
# =============================================================================
# Cài Docker Engine + Buildx ARM64 trong WSL Ubuntu-24.04
# Chạy: bash deploy/setup/install_docker_wsl.sh
# =============================================================================
set -e
Y='\033[1;33m'; G='\033[0;32m'; R='\033[0;31m'; B='\033[0;34m'; NC='\033[0m'
step() { echo -e "${Y}[+] $1${NC}"; }
ok()   { echo -e "${G}[OK] $1${NC}"; }
err()  { echo -e "${R}[ERR] $1${NC}"; exit 1; }

echo -e "${B}======================================================"
echo "  Docker Engine + Buildx ARM64 Setup (WSL Ubuntu)"
echo -e "======================================================${NC}"

# ── 1. Gỡ bỏ phiên bản cũ nếu có ─────────────────────────────────────────────
step "[1/7] Removing old Docker packages..."
for pkg in docker.io docker-doc docker-compose docker-compose-v2 podman-docker containerd runc; do
    sudo apt-get remove -y $pkg 2>/dev/null || true
done
ok "Old packages removed"

# ── 2. Cài các gói phụ thuộc + GPG key chính thức ────────────────────────────
step "[2/7] Adding Docker official GPG key..."
sudo apt-get update -qq
sudo apt-get install -y --no-install-recommends ca-certificates curl gnupg
sudo install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | \
    sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
sudo chmod a+r /etc/apt/keyrings/docker.gpg
ok "GPG key added"

# ── 3. Thêm Docker repository ─────────────────────────────────────────────────
step "[3/7] Adding Docker apt repository..."
echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] \
https://download.docker.com/linux/ubuntu $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
    sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update -qq
ok "Repository added"

# ── 4. Cài Docker Engine + buildx + compose ───────────────────────────────────
step "[4/7] Installing Docker Engine, Buildx, Compose..."
sudo apt-get install -y --no-install-recommends \
    docker-ce docker-ce-cli containerd.io \
    docker-buildx-plugin docker-compose-plugin
ok "Docker installed"

# ── 5. Thêm user vào group docker ────────────────────────────────────────────
step "[5/7] Adding $USER to docker group..."
sudo usermod -aG docker "$USER"
ok "User $USER added to docker group"

# ── 6. Khởi động Docker daemon ngay trong WSL ─────────────────────────────────
step "[6/7] Starting Docker daemon..."
sudo service docker start
# Chờ daemon sẵn sàng (tối đa 15 giây)
for i in $(seq 1 15); do
    if sudo docker info >/dev/null 2>&1; then
        ok "Docker daemon is running"
        break
    fi
    [ $i -eq 15 ] && err "Docker daemon did not start in time!"
    sleep 1
done

# ── 7. Cài QEMU binfmt + tạo buildx builder ARM64 ───────────────────────────
step "[7/7] Installing QEMU binfmt + creating drone-builder..."

# Cài QEMU binfmt (để build ARM64 trên x86_64)
sudo apt-get install -y --no-install-recommends qemu-user-static binfmt-support
sudo docker run --privileged --rm tonistiigi/binfmt --install arm64,arm 2>/dev/null || true

# Tạo buildx builder với docker-container driver
sudo docker buildx rm drone-builder 2>/dev/null || true
sudo docker buildx create --name drone-builder --driver docker-container --bootstrap
ok "drone-builder created"

# ── Cấu hình auto-start Docker trong WSL (via /etc/wsl.conf) ─────────────────
if ! grep -q "command = service docker start" /etc/wsl.conf 2>/dev/null; then
    step "Configuring Docker auto-start in /etc/wsl.conf..."
    sudo tee -a /etc/wsl.conf > /dev/null <<'EOF'

[boot]
command = service docker start
EOF
    ok "Auto-start configured (hiệu lực sau khi restart WSL)"
fi

# ── Kiểm tra kết quả ─────────────────────────────────────────────────────────
echo ""
echo -e "${B}======================================================"
echo "  Kết quả:"
echo -e "======================================================${NC}"
sudo docker --version
sudo docker buildx version
sudo docker buildx ls | grep drone-builder
echo ""
echo -e "${G}✅ Docker Engine đã sẵn sàng build ARM64!${NC}"
echo ""
echo -e "${Y}⚠️  Vì chạy bằng sudo, hãy logout + login WSL để dùng docker không cần sudo.${NC}"
echo "   Hoặc dùng lệnh tạm thời ngay bây giờ:"
echo ""
echo -e "${B}   newgrp docker${NC}"
echo ""
echo "  Sau đó build + deploy:"
echo -e "${B}   cd ~/Drone && make deploy${NC}"
