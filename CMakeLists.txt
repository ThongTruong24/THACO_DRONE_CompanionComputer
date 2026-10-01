cmake_minimum_required(VERSION 3.20)
project(DroneProject VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to enable or disable tests
option(BUILD_TESTING "Build test suite" ON)

if(BUILD_TESTING)
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)
endif()

# Core native modules
add_subdirectory(src/mavlink-router-controller)
add_subdirectory(src/cc-agent)
add_subdirectory(src/camera-stream-controller)

# System integration tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
