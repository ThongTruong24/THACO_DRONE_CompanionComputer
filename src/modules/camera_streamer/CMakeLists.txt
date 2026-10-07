cmake_minimum_required(VERSION 3.20)
project(camera_streamer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3 -Wall -Wextra -pthread)

# ARM NEON optimization for ARM64 (Raspberry Pi 5)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    add_compile_options(-march=armv8-a+crc -mtune=cortex-a76)
    message(STATUS "Kích hoạt tối ưu hóa phần cứng Cortex-A76 & NEON cho Pi 5")
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cc_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cc_frame_pool REQUIRED)
find_package(cc_systemlib REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)

# Check librealsense2
find_path(REALSENSE_INCLUDE_DIR librealsense2/rs.hpp
    PATHS
    /opt/ros/jazzy/include
    /usr/include
    /usr/local/include
)

find_library(REALSENSE_LIB
    NAMES realsense2
    PATHS
    /opt/ros/jazzy/lib
    /opt/ros/jazzy/lib/aarch64-linux-gnu   
    /opt/ros/jazzy/lib/x86_64-linux-gnu
    /usr/lib/aarch64-linux-gnu
    /usr/lib/x86_64-linux-gnu
    /usr/lib
    /usr/local/lib
)

set(EXTRA_LIBS pthread)

if(REALSENSE_INCLUDE_DIR AND REALSENSE_LIB)
    message(STATUS "Tìm thấy librealsense2 tại: ${REALSENSE_INCLUDE_DIR}, lib: ${REALSENSE_LIB}")
    add_definitions(-DHAVE_REALSENSE=1)
    include_directories(${REALSENSE_INCLUDE_DIR})
    list(APPEND EXTRA_LIBS ${REALSENSE_LIB})
else()
    message(WARNING "Không tìm thấy librealsense2 trên host. Biên dịch chế độ Stub (hỗ trợ V4L2/Standby Camera).")
    add_definitions(-DHAVE_REALSENSE=0)
endif()

set(CC_DRIVERS_CAMERA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../drivers/camera)
set(HRT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/hrt/include)
set(CC_FRAME_POOL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/frame_pool/include)

add_library(camera_streamer_core
    src/ai_branch.cpp
    src/gst_stream_sink.cpp
    src/neon_processor.cpp
    src/camera_streamer_node.cpp
    ${CC_DRIVERS_CAMERA_DIR}/realsense/realsense_camera.cpp
    ${CC_DRIVERS_CAMERA_DIR}/v4l2/v4l2_camera.cpp
)

target_include_directories(camera_streamer_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${HRT_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CC_FRAME_POOL_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CC_DRIVERS_CAMERA_DIR}>
    $<BUILD_INTERFACE:${CC_DRIVERS_CAMERA_DIR}/v4l2>
    $<BUILD_INTERFACE:${CC_DRIVERS_CAMERA_DIR}/realsense>
    $<INSTALL_INTERFACE:include>
    ${GST_INCLUDE_DIRS}
)

ament_target_dependencies(camera_streamer_core
    rclcpp
    cc_msgs
    sensor_msgs
    cc_frame_pool
    cc_systemlib
)

target_link_libraries(camera_streamer_core
    ${GST_LIBRARIES}
    ${EXTRA_LIBS}
)

add_executable(camera_streamer_node
    src/main.cpp
)

target_link_libraries(camera_streamer_node PRIVATE
    camera_streamer_core
)

install(TARGETS camera_streamer_node camera_streamer_core
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY config/
    DESTINATION share/${PROJECT_NAME}/config
)

install(DIRECTORY ${CC_DRIVERS_CAMERA_DIR}/config/
    DESTINATION share/${PROJECT_NAME}/config/cameras
)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

ament_package()
