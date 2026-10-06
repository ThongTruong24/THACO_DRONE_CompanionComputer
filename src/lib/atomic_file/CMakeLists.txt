cmake_minimum_required(VERSION 3.20)
project(cc_atomic_file)

add_library(cc_atomic_file INTERFACE)
target_include_directories(cc_atomic_file INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
