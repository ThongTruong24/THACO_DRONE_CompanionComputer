# ============================================================
# Makefile - Drone Edge Computing (Raspberry Pi 5)
# Quản lý các Module Độc Lập:
#   Core: MAVLink Router, CC Agent + Hardware Registry, Networking, Camera
#   Optional: Vision and MAVROS (Compose service mavros, image drone-mavros)
# ============================================================

-include .env

PI_HOST ?= $(shell bash deploy/ship/resolve_target.sh --quiet)
PI_USER ?= thong
REGISTRY ?= ghcr.io
IMAGE_NAMESPACE ?=
IMAGE_TAG ?= dev-ai-tracking-flow
EXPECTED_BRANCH ?= dev/ai_tracking_flow
BUILDER ?= drone-builder
export REGISTRY IMAGE_NAMESPACE IMAGE_TAG EXPECTED_BRANCH BUILDER

.PHONY: all help build build-mavlink build-cc-agent build-networking build-camera-rtsp build-vision build-mavros build-all \
        deploy deploy-fast deploy-mavlink deploy-cc-agent deploy-networking deploy-camera-rtsp deploy-vision deploy-mavros \
        start stop restart ps logs logs-mavlink logs-cc-agent logs-networking logs-camera-rtsp logs-vision logs-mavros logs-all \
        ssh setup-pi autostart doctor check clean build-native test

all: help

help:
	@echo ""
	@echo "┌─────────────────────────────────────────────────────────────────┐"
	@echo "│  Drone Edge - Modular Flight Stack & Dual-Target C++ System     │"
	@echo "└─────────────────────────────────────────────────────────────────┘"
	@echo ""
	@echo "  🛠️  CÀI ĐẶT BAN ĐẦU:"
	@echo "    make setup-ssh          Cấu hình SSH Key tới Pi (chạy 1 lần đầu)"
	@echo "    make setup-pi           Cài Docker + config UART4 trên Pi"
	@echo "    make autostart          Bật tự động chạy khi cắm nguồn Pi (Systemd)"
	@echo ""
	@echo "  🧪 KIỂM THỬ TẬP TRUNG (GTEST & CTEST):"
	@echo "    make build-native       Biên dịch toàn bộ C++ Native modules & Tests"
	@echo "    make test               Chạy toàn bộ 30 Unit & Integration Tests (100% C++)"
	@echo ""
	@echo "  📦 BUILD TỪNG CONTAINER (WSL x86_64 → linux/arm64):"
	@echo "    make build-mavlink      Build ARM64 image mavlink-router-controller"
	@echo "    make build-cc-agent     Build ARM64 image cc-agent"
	@echo "    make build-networking   Build ARM64 image drone-networking"
	@echo "    make build-camera-rtsp  Build ARM64 image camera-stream-controller"
	@echo "    make build-mavros       Build ARM64 image drone-mavros"
	@echo "    make build-vision       Build ARM64 image drone-vision"
	@echo "    make build-all          Build 4 core ARM64 images local (--load)"
	@echo ""
	@echo "  🚀 DEPLOY TỪNG CONTAINER LÊN PI:"
	@echo "    make deploy-mavlink     Deploy mavlink-router-controller"
	@echo "    make deploy-cc-agent    Deploy cc-agent"
	@echo "    make deploy-networking  Deploy drone-networking"
	@echo "    make deploy-camera-rtsp Deploy camera-stream-controller"
	@echo "    make deploy-mavros      Deploy service mavros (image drone-mavros)"
	@echo "    make deploy-vision      Deploy drone-vision"
	@echo "    make publish-core       Build + push 4 core images lên GHCR"
	@echo "    make update             Trên Pi: Git pull + image pull + up --no-build"
	@echo "    make deploy             Publish + Git/registry update qua SSH"
	@echo "    make deploy-fast        Git/registry update qua SSH; không publish"
	@echo ""
	@echo "  ⚡ BẬT / TẮT / KHỞI ĐỘNG LẠI (trên Pi):"
	@echo "    make start-all / stop-all / restart-all"
	@echo "    make restart-mavlink / restart-cc-agent"
	@echo ""
	@echo "  📊 GIÁM SÁT & LOGS:"
	@echo "    make ps                 Kiểm tra trạng thái các container trên Pi"
	@echo "    make logs-mavlink       Xem log MAVLink Router từ Pi"
	@echo "    make doctor             Chẩn đoán hệ thống tự động toàn diện"
	@echo ""

# ─── C++ Native Build & Testing (CTest / GoogleTest) ─────────────────────
build-native:
	@echo "▶ Building all C++ native modules..."
	@mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON .. && make -j4
	@echo "✓ Native build complete!"

test: build-native
	@echo "▶ Running all C++ Unit & Integration Tests via CTest..."
	@cd build && ctest --output-on-failure

# ─── ARM64 images: local build or GHCR publication ──────────────────────
.PHONY: build-mavlink
build-mavlink:
	@bash deploy/build/images.sh build mavlink-router-controller

.PHONY: publish-mavlink
publish-mavlink:
	@bash deploy/build/images.sh publish mavlink-router-controller

.PHONY: build-cc-agent
build-cc-agent:
	@bash deploy/build/images.sh build cc-agent

.PHONY: publish-cc-agent
publish-cc-agent:
	@bash deploy/build/images.sh publish cc-agent

.PHONY: build-networking
build-networking:
	@bash deploy/build/images.sh build drone-networking

.PHONY: publish-networking
publish-networking:
	@bash deploy/build/images.sh publish drone-networking

.PHONY: build-camera-rtsp
build-camera-rtsp:
	@bash deploy/build/images.sh build camera-stream-controller

.PHONY: publish-camera-rtsp
publish-camera-rtsp:
	@bash deploy/build/images.sh publish camera-stream-controller

.PHONY: build-mavros
build-mavros:
	@bash deploy/build/images.sh build mavros

.PHONY: publish-mavros
publish-mavros:
	@bash deploy/build/images.sh publish mavros

.PHONY: build-vision-base
build-vision-base:
	@bash deploy/build/images.sh build vision-base

.PHONY: publish-vision-base
publish-vision-base:
	@bash deploy/build/images.sh publish vision-base

.PHONY: build-vision
build-vision:
	@bash deploy/build/images.sh build drone-vision

.PHONY: publish-vision
publish-vision:
	@bash deploy/build/images.sh publish drone-vision

.PHONY: build-core
build-core:
	@bash deploy/build/images.sh build core

.PHONY: publish-core
publish-core:
	@bash deploy/build/images.sh publish core

.PHONY: build-all publish publish-all update deploy-offline
build-all: build-core
publish publish-all: publish-core
build-hw build-hardware-manager: build-cc-agent
build-wifi-ap: build-networking
build-camera: build-camera-rtsp

update:
	@bash deploy/update.sh

deploy-offline:
	@bash deploy/ship/offline-deploy.sh $(PI_USER)@$(PI_HOST)

# ─── Deploy ─────────────────────────────────────────────────────────────
deploy:
	@bash deploy/ship/deploy.sh $(PI_USER)@$(PI_HOST)

deploy-fast:
	@bash deploy/ship/deploy.sh --no-publish $(PI_USER)@$(PI_HOST)



deploy-mavlink:
	@bash deploy/ship/deploy.sh mavlink-router-controller $(PI_USER)@$(PI_HOST)

deploy-cc-agent:
	@bash deploy/ship/deploy.sh cc-agent $(PI_USER)@$(PI_HOST)

deploy-networking:
	@bash deploy/ship/deploy.sh drone-networking $(PI_USER)@$(PI_HOST)

deploy-wifi-ap: deploy-networking

deploy-camera-rtsp:
	@bash deploy/ship/deploy.sh camera-stream-controller $(PI_USER)@$(PI_HOST)

deploy-camera: deploy-camera-rtsp

deploy-mavros:
	@bash deploy/ship/deploy.sh mavros $(PI_USER)@$(PI_HOST)

deploy-vision:
	@bash deploy/ship/deploy.sh drone-vision $(PI_USER)@$(PI_HOST)

# ─── Bật / Tắt / Khởi động lại trên Pi ──────────────────────────────────

start-mavlink:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml up -d --no-build mavlink-router-controller"

stop-mavlink:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml stop mavlink-router-controller"

restart-mavlink:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml restart mavlink-router-controller"

start-cc-agent:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml up -d --no-build cc-agent"

stop-cc-agent:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml stop cc-agent"

restart-cc-agent:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml restart cc-agent"

start-networking:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml up -d --no-build drone-networking"

stop-networking:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml stop drone-networking"

restart-networking:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml restart drone-networking"

start-wifi-ap: start-networking
stop-wifi-ap: stop-networking
restart-wifi-ap: restart-networking

start-camera-rtsp:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml up -d --no-build camera-stream-controller"

stop-camera-rtsp:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml stop camera-stream-controller"

restart-camera-rtsp:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml restart camera-stream-controller"

start-vision:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml up -d --no-build drone-vision"

stop-vision:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml stop drone-vision"

restart-vision:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml restart drone-vision"

start-mavros:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml --profile mavros up -d --no-build mavros"

stop-mavros:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml --profile mavros stop mavros"

restart-mavros:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml --profile mavros restart mavros"

start-all:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml up -d --no-build"

stop-all:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml stop"

restart-all:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml restart"

# ─── Giám sát & Logs ────────────────────────────────────────────────────
ps:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml ps"

log-mavlink:
	@ssh -t $(PI_USER)@$(PI_HOST) "bash ~/drone-edge/deploy/monitor/mavlink-status.sh $(MODE)"

logs-mavlink: log-mavlink
log-mavlink-ports:
	@ssh -t $(PI_USER)@$(PI_HOST) "bash ~/drone-edge/deploy/monitor/mavlink-status.sh 1"
log-mavlink-stats:
	@ssh -t $(PI_USER)@$(PI_HOST) "bash ~/drone-edge/deploy/monitor/mavlink-status.sh 2"


logs-cc-agent:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml logs -f --tail=100 cc-agent"

logs-networking:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml logs -f --tail=100 drone-networking"

logs-wifi-ap: logs-networking

logs-camera-rtsp:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml logs -f --tail=100 camera-stream-controller"

logs-vision:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml logs -f --tail=100 drone-vision"

logs-mavros:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml --profile mavros logs -f --tail=100 mavros"

logs-all:
	@ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml logs -f --tail=50"

# ─── Setup Pi & System ──────────────────────────────────────────────────
setup:
	@bash deploy/setup/setup.sh --host $(PI_HOST) --user $(PI_USER)

setup-ssh:
	@bash deploy/setup/setup.sh --ssh --host $(PI_HOST) --user $(PI_USER)

setup-pi:
	@bash deploy/setup/setup.sh --host $(PI_HOST) --user $(PI_USER)

autostart:
	@echo "▶ Cấu hình Systemd Autostart trên Pi ($(PI_HOST))..."
	@scp deploy/systemd/drone-edge.service $(PI_USER)@$(PI_HOST):/tmp/
	@ssh -t $(PI_USER)@$(PI_HOST) "\
		sudo sed -i 's|WorkingDirectory=.*|WorkingDirectory=/home/$(PI_USER)/drone-edge|' /tmp/drone-edge.service && \
		sudo cp /tmp/drone-edge.service /etc/systemd/system/drone-edge.service && \
		sudo systemctl daemon-reload && \
		sudo systemctl enable drone-edge.service && \
		echo '✓ Đã kích hoạt Autostart drone-edge.service thành công!' && \
		systemctl is-enabled drone-edge.service"

doctor:
	@echo "▶ Đang chạy chẩn đoán tuần tự trên Pi ($(PI_HOST))..."
	@ssh -t $(PI_USER)@$(PI_HOST) "mkdir -p ~/drone-edge/deploy/monitor"
	@scp deploy/monitor/doctor.sh $(PI_USER)@$(PI_HOST):~/drone-edge/deploy/monitor/
	@ssh -t $(PI_USER)@$(PI_HOST) "bash ~/drone-edge/deploy/monitor/doctor.sh"

check: doctor

perf:
	@ssh -t $(PI_USER)@$(PI_HOST) "bash ~/drone-edge/deploy/monitor/perf.sh"

clean:
	@echo "Use docker image rm with explicit GHCR image refs to remove local artifacts."
	@docker buildx rm drone-builder 2>/dev/null || true
	@echo "✓ Cleaned"

apply-netplan:
	@echo "▶ Áp dụng cấu hình Netplan từ src/drone-networking/50-drone.yaml lên Pi ($(PI_HOST))..."
	@scp src/drone-networking/50-drone.yaml $(PI_USER)@$(PI_HOST):/tmp/50-drone.yaml
	@ssh -t $(PI_USER)@$(PI_HOST) "sudo cp /tmp/50-drone.yaml /etc/netplan/ && sudo netplan apply && echo '✓ Cấu hình mạng đã áp dụng thành công!'"

clean-pi:
	@echo "▶ Đang dọn dẹp Docker images rác và build cache trên Pi ($(PI_USER)@$(PI_HOST))..."
	@ssh $(PI_USER)@$(PI_HOST) "docker system prune -f"
	@echo "✓ Thu hồi bộ nhớ trên Raspberry Pi hoàn tất!"
