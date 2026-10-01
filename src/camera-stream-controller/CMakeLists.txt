cmake_minimum_required(VERSION 3.16)
project(drone_camera_streamer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3 -Wall -Wextra -pthread)

# Tối ưu hóa vi kiến trúc ARM NEON nếu chạy trên ARM64 (Raspberry Pi 5)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    add_compile_options(-march=armv8-a+crc -mtune=cortex-a76)
    message(STATUS "Kích hoạt tối ưu hóa phần cứng Cortex-A76 & NEON cho Pi 5")
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
pkg_check_modules(YAML REQUIRED yaml-cpp)

# Kiểm tra sự tồn tại của thư viện librealsense2
find_path(REALSENSE_INCLUDE_DIR librealsense2/rs.hpp
    PATHS
    /opt/ros/jazzy/include
    /usr/include
    /usr/local/include
)

set(EXTRA_LIBS pthread)

if(REALSENSE_INCLUDE_DIR)
    message(STATUS "Tìm thấy librealsense2 tại: ${REALSENSE_INCLUDE_DIR}")
    add_definitions(-DHAVE_REALSENSE=1)
    include_directories(${REALSENSE_INCLUDE_DIR})
    link_directories(
        /opt/ros/jazzy/lib/aarch64-linux-gnu
        /opt/ros/jazzy/lib
        /usr/lib/aarch64-linux-gnu
    )
    list(APPEND EXTRA_LIBS realsense2)
else()
    message(WARNING "Không tìm thấy librealsense2 trên host. Biên dịch chế độ Stub (hỗ trợ V4L2/Standby Camera).")
    add_definitions(-DHAVE_REALSENSE=0)
endif()

include_directories(
    include
    ${GST_INCLUDE_DIRS}
    ${YAML_INCLUDE_DIRS}
)

link_directories(
    ${GST_LIBRARY_DIRS}
    ${YAML_LIBRARY_DIRS}
)

add_executable(drone_camera_streamer
    src/main.cpp
    src/config_manager.cpp
    src/neon_processor.cpp
    src/standby_generator.cpp
    src/realsense_camera.cpp
    src/v4l2_camera.cpp
    src/gst_stream_sink.cpp
)

target_link_libraries(drone_camera_streamer
    ${GST_LIBRARIES}
    ${YAML_LIBRARIES}
    ${EXTRA_LIBS}
)

install(TARGETS drone_camera_streamer DESTINATION /usr/local/bin)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
