#!/usr/bin/env bash
# ==============================================================================
# Drone Project - MAVLink Definitions Updater & Synchronizer
# Location: deploy/monitor/update_mavlink.sh
# Author: THACO AgriDrone
# ==============================================================================

set -euo pipefail

# ANSI colors
GREEN="\033[92m"
BLUE="\033[94m"
YELLOW="\033[93m"
RED="\033[91m"
CYAN="\033[96m"
BOLD="\033[1m"
RESET="\033[0m"

echo -e "\n${BOLD}${CYAN}==================================================================${RESET}"
echo -e "${BOLD}${CYAN}  Drone Edge - MAVLink Definitions & Code Sync                    ${RESET}"
echo -e "${BOLD}${CYAN}==================================================================${RESET}\n"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DRONE_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
SSOT_ROOT="${MAVLINK_SSOT_ROOT:-$(cd "${DRONE_ROOT}/.." && pwd)/Mavlink}"
SSOT_XML="$SSOT_ROOT/custom/thaco_common.xml"
SSOT_SYNC="$SSOT_ROOT/sync.sh"

LOCAL_XML="${DRONE_ROOT}/src/mavros/mavlink/agridrone-thaco-mavlink/message_definitions/v1.0/thaco_common.xml"
CPP_AGENT_MAVLINK="${DRONE_ROOT}/src/cc-agent/mavlink/thaco_common"

if [ ! -d "$SSOT_ROOT" ]; then
    echo -e "${RED}[ERROR] Single Source of Truth not found at $SSOT_ROOT!${RESET}"
    exit 1
fi

# Check if local XML was modified differently from SSOT
if [ -f "$LOCAL_XML" ] && [ -f "$SSOT_XML" ]; then
    if ! cmp -s "$LOCAL_XML" "$SSOT_XML"; then
        echo -e "${YELLOW}[WARN] Difference detected between local Drone XML and SSOT XML!${RESET}"
        if [ "$LOCAL_XML" -nt "$SSOT_XML" ]; then
            echo -e "${CYAN}[INFO] Local Drone XML is newer. Updating SSOT ($SSOT_XML)...${RESET}"
            cp "$LOCAL_XML" "$SSOT_XML"
        else
            echo -e "${CYAN}[INFO] SSOT XML is newer. Updating local Drone XML from SSOT...${RESET}"
            cp "$SSOT_XML" "$LOCAL_XML"
        fi
    fi
fi

# Run SSOT generator
echo -e "${BLUE}[INFO] Executing MAVLink SSOT Generator...${RESET}"
bash "$SSOT_SYNC"

# Verification
HEADER_COUNT=$(ls -1 "$CPP_AGENT_MAVLINK"/*.h 2>/dev/null | wc -l || echo 0)
echo -e "\n${GREEN}[SUCCESS] Drone MAVLink synchronizer completed successfully!${RESET}"
echo -e "   • C++ Companion Agent headers: ${BOLD}$HEADER_COUNT headers${RESET} in $CPP_AGENT_MAVLINK"
echo -e "   • C++ Companion Agent: native C++20 (drone-companion-agent)"
echo -e "   • Authoritative definition: $SSOT_XML\n"
