# ============================================================
# Makefile - Drone Edge Computing (Raspberry Pi 5)
# Service = container = image = edge-*, mỗi service là một module:
#   router  -> edge-router   (src/modules/mavlink_router)
#   agent   -> edge-agent    (src/modules/cc_agent_legacy, thay bằng edge_agent ở T12)
#   network -> edge-network  (src/modules/networking)
#   camera  -> edge-camera   (src/modules/camera_streamer)
#   vision    -> edge-vision     (src/modules/vision)
#   vision-v1 -> edge-vision-v1  (src/modules/vision_v1)
# Build context của mọi Dockerfile là thư mục gốc repo (.).
# ============================================================

-include .env

PI_HOST := $(shell bash deploy/ship/resolve_target.sh --quiet)
PI_USER ?= thong

SERVICES := router agent network camera vision vision-v1
MODULE_router  := src/modules/mavlink_router
MODULE_agent   := platforms/linux/edge_agent
MODULE_network := src/modules/networking
MODULE_camera  := src/modules/camera_streamer
MODULE_vision  := src/modules/vision
MODULE_vision-v1 := src/modules/vision_v1

BUILD_TARGETS   := $(addprefix build-,$(SERVICES))
DEPLOY_TARGETS  := $(addprefix deploy-,$(SERVICES))
START_TARGETS   := $(addprefix start-,$(SERVICES))
STOP_TARGETS    := $(addprefix stop-,$(SERVICES))
RESTART_TARGETS := $(addprefix restart-,$(SERVICES))
LOGS_TARGETS    := $(addprefix logs-,$(SERVICES))
GENERIC_START_TARGETS := $(filter-out start-vision start-vision-v1,$(START_TARGETS))
GENERIC_RESTART_TARGETS := $(filter-out restart-vision restart-vision-v1,$(RESTART_TARGETS))

COMPOSE_REMOTE = ssh $(PI_USER)@$(PI_HOST) "docker compose -f ~/drone-edge/docker-compose.yml

.PHONY: all help build-all deploy deploy-fast start-all stop-all restart-all ps logs-all \
        $(BUILD_TARGETS) $(DEPLOY_TARGETS) $(START_TARGETS) $(STOP_TARGETS) $(RESTART_TARGETS) $(LOGS_TARGETS) \
        log-router-ports log-router-stats \
        setup setup-ssh setup-pi autostart doctor check perf clean clean-pi apply-netplan \
        ensure-builder build-native test update-mavlink

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
	@echo "  🧪 KIỂM THỬ TẬP TRUNG (colcon + GoogleTest):"
	@echo "    make build-native       colcon build các package C++ (colcon_defaults.yaml)"
	@echo "    make test               colcon test + colcon test-result"
	@echo "    make update-mavlink     Sinh lại headers MAVLink vào src/lib/mavlink (../sync_mavlink.py từ Drone_MAVLink/thaco.xml)"
	@echo ""
	@echo "  📦 BUILD TỪNG CONTAINER (trên WSL ARM64), <svc> = router|agent|network|camera|vision|vision-v1:"
	@echo "    make build-<svc>        Build ARM64 image edge-<svc>"
	@echo "    make build-all          Build các container cốt lõi (router agent network camera)"
	@echo ""
	@echo "  🚀 DEPLOY LÊN PI:"
	@echo "    make deploy-<svc>       Deploy edge-<svc>"
	@echo "    make deploy             Deploy toàn bộ hệ thống"
	@echo "    make deploy-fast        Deploy toàn bộ không build lại"
	@echo ""
	@echo "  ⚡ BẬT / TẮT / KHỞI ĐỘNG LẠI (trên Pi):"
	@echo "    make start-<svc> / stop-<svc> / restart-<svc>"
	@echo "    make start-all / stop-all / restart-all"
	@echo ""
	@echo "  📊 GIÁM SÁT & LOGS:"
	@echo "    make ps                 Kiểm tra trạng thái các container trên Pi"
	@echo "    make logs-<svc>         Xem log edge-<svc> từ Pi (logs-router: trạng thái MAVLink Router)"
	@echo "    make doctor             Chẩn đoán hệ thống tự động toàn diện"
	@echo ""

# ─── MAVLink header regeneration (src/lib/mavlink) ───────────────────────
update-mavlink:
	@test -f ../sync_mavlink.py || { echo "Run inside the THACO_Drone superproject (needs ../sync_mavlink.py)"; exit 1; }
	@cd .. && python3 sync_mavlink.py

# ─── C++ Native Build & Testing (colcon / GoogleTest) ────────────────────
build-native:
	@echo "▶ Building all C++ native packages (colcon)..."
	@bash -c 'source /opt/ros/jazzy/setup.bash && COLCON_DEFAULTS_FILE=$(CURDIR)/colcon_defaults.yaml colcon build'
	@echo "✓ Native build complete!"

test: build-native
	@echo "▶ Running all C++ Unit & Integration Tests via colcon test..."
	@bash -c 'source /opt/ros/jazzy/setup.bash && COLCON_DEFAULTS_FILE=$(CURDIR)/colcon_defaults.yaml colcon test && colcon test-result --verbose'

format:
	@echo "▶ Formatting code using PX4 astyle..."
	@bash Tools/astyle/check_code_style_all.sh --fix

check_format:
	@echo "▶ Checking code formatting..."
	@bash Tools/astyle/check_code_style_all.sh

# ─── Build ARM64 Docker Images ──────────────────────────────────────────
ensure-builder:
	@if ! docker buildx inspect drone-builder >/dev/null 2>&1; then \
		docker buildx create --name drone-builder --driver docker-container --bootstrap; \
	elif ! docker buildx inspect drone-builder --bootstrap >/dev/null 2>&1; then \
		echo "⚡ Recreating stale drone-builder buildx instance..."; \
		docker buildx rm -f drone-builder >/dev/null 2>&1 || true; \
		docker buildx create --name drone-builder --driver docker-container --bootstrap; \
	fi

$(BUILD_TARGETS): build-%: ensure-builder
	@echo "▶ Building edge-$*:latest..."
	@docker buildx build --builder drone-builder --platform linux/arm64 --tag edge-$*:latest --file $(MODULE_$*)/Dockerfile --load .
	@echo "✓ Build edge-$* hoàn tất!"

build-all: ensure-builder
	@echo "▶ Building core containers using docker buildx bake..."
	@docker buildx bake -f docker-bake.hcl --builder drone-builder --load
	@echo "✓ Build toàn bộ container cốt lõi hoàn tất!"

# ─── Deploy ─────────────────────────────────────────────────────────────
deploy:
	@bash deploy/ship/deploy.sh $(PI_USER)@$(PI_HOST)

deploy-fast:
	@bash deploy/ship/deploy.sh --no-build $(PI_USER)@$(PI_HOST)

$(DEPLOY_TARGETS): deploy-%:
	@bash deploy/ship/deploy.sh edge-$* $(PI_USER)@$(PI_HOST)

registry-start:
	@if ! docker ps --format '{{.Names}}' | grep -q '^cc-registry$$'; then \
		if docker ps -a --format '{{.Names}}' | grep -q '^cc-registry$$'; then \
			docker start cc-registry >/dev/null; \
		else \
			docker run -d -p 5000:5000 --name cc-registry --restart=always registry:2 >/dev/null; \
		fi; \
	fi
	@echo "✓ Local Docker Registry đang chạy trên cổng 5000"

registry-stop:
	@docker stop cc-registry >/dev/null 2>&1 || true
	@echo "✓ Local Docker Registry đã dừng"

# ─── Bật / Tắt / Khởi động lại trên Pi ──────────────────────────────────
$(GENERIC_START_TARGETS): start-%:
	@$(COMPOSE_REMOTE) up -d edge-$*"

start-vision:
	@$(COMPOSE_REMOTE) stop edge-vision-v1 >/dev/null 2>&1 || true; docker compose -f ~/drone-edge/docker-compose.yml up -d edge-vision"

start-vision-v1:
	@$(COMPOSE_REMOTE) stop edge-vision >/dev/null 2>&1 || true; docker compose -f ~/drone-edge/docker-compose.yml up -d edge-vision-v1"

$(STOP_TARGETS): stop-%:
	@$(COMPOSE_REMOTE) stop edge-$*"

$(GENERIC_RESTART_TARGETS): restart-%:
	@$(COMPOSE_REMOTE) restart edge-$*"

restart-vision: stop-vision start-vision

restart-vision-v1: stop-vision-v1 start-vision-v1

start-all:
	@$(COMPOSE_REMOTE) up -d"

stop-all:
	@$(COMPOSE_REMOTE) stop"

restart-all:
	@$(COMPOSE_REMOTE) restart"

# ─── Giám sát & Logs ────────────────────────────────────────────────────
ps:
	@$(COMPOSE_REMOTE) ps"

logs-router:
	@ssh -t $(PI_USER)@$(PI_HOST) "bash ~/drone-edge/deploy/monitor/mavlink-status.sh $(MODE)"
log-router-ports:
	@ssh -t $(PI_USER)@$(PI_HOST) "bash ~/drone-edge/deploy/monitor/mavlink-status.sh 1"
log-router-stats:
	@ssh -t $(PI_USER)@$(PI_HOST) "bash ~/drone-edge/deploy/monitor/mavlink-status.sh 2"

$(filter-out logs-router,$(LOGS_TARGETS)): logs-%:
	@$(COMPOSE_REMOTE) logs -f --tail=100 edge-$*"

logs-all:
	@$(COMPOSE_REMOTE) logs -f --tail=50"

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
	@docker rmi $(addsuffix :latest,$(addprefix edge-,$(SERVICES))) 2>/dev/null || true
	@docker buildx rm drone-builder 2>/dev/null || true
	@echo "✓ Cleaned"

apply-netplan:
	@echo "▶ Áp dụng cấu hình Netplan từ $(MODULE_network)/50-drone.yaml lên Pi ($(PI_HOST))..."
	@scp $(MODULE_network)/50-drone.yaml $(PI_USER)@$(PI_HOST):/tmp/50-drone.yaml
	@ssh -t $(PI_USER)@$(PI_HOST) "sudo cp /tmp/50-drone.yaml /etc/netplan/ && sudo netplan apply && echo '✓ Cấu hình mạng đã áp dụng thành công!'"

clean-pi:
	@echo "▶ Đang dọn dẹp Docker images rác và build cache trên Pi ($(PI_USER)@$(PI_HOST))..."
	@ssh $(PI_USER)@$(PI_HOST) "docker system prune -f"
	@echo "✓ Thu hồi bộ nhớ trên Raspberry Pi hoàn tất!"
