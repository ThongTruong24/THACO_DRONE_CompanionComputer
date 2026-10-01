cmake_minimum_required(VERSION 3.16)
project(drone_companion_agent VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -pthread")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/mavlink/thaco_common
    ${CMAKE_CURRENT_SOURCE_DIR}/mavlink
)

# Source files (Clean SOLID structure, zero hardcoded paths)
set(SOURCES
    src/app_paths.cpp
    src/hardware_registry.cpp
    src/uds_hub_server.cpp
    src/system_metrics.cpp
    src/config_engine.cpp
    src/rollback_guard.cpp
    src/network_monitor.cpp
    src/mavlink_core.cpp
    src/main.cpp
)

add_executable(drone-companion-agent ${SOURCES})
target_link_libraries(drone-companion-agent PRIVATE pthread)

install(TARGETS drone-companion-agent DESTINATION /usr/local/bin)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
