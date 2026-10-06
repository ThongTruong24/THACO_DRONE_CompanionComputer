cmake_minimum_required(VERSION 3.20)
project(mavlink_router VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -O2 -pthread)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cc_msgs REQUIRED)
find_package(cc_link_config REQUIRED)
find_package(cc_serial REQUIRED)

set(HRT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/hrt/include)

add_library(mavlink_router_core
    src/router_config_generator.cpp
    src/router_stats_parser.cpp
    src/serial_link_monitor.cpp
    src/mavlink_router.cpp
    src/mavlink_router_node.cpp
)

target_include_directories(mavlink_router_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${HRT_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(mavlink_router_core
    rclcpp
    cc_msgs
    cc_link_config
    cc_serial
)

add_executable(mavlink-router-controller
    src/main.cpp
)
target_link_libraries(mavlink-router-controller PRIVATE
    mavlink_router_core
    pthread
)

install(TARGETS mavlink-router-controller mavlink_router_core
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)

if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)
    add_subdirectory(tests)
endif()

ament_export_include_directories(include)
ament_export_libraries(mavlink_router_core)
ament_export_dependencies(rclcpp cc_msgs cc_link_config cc_serial)
ament_package()
