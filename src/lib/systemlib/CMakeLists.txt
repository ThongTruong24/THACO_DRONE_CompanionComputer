cmake_minimum_required(VERSION 3.20)
project(cc_systemlib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cc_msgs REQUIRED)

add_library(cc_systemlib INTERFACE)
target_include_directories(cc_systemlib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../hrt/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(cc_systemlib INTERFACE
    rclcpp::rclcpp
    ${cc_msgs_TARGETS}
)

if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)
    ament_add_gtest(unit_mavlink_log tests/test_mavlink_log.cpp)
    target_link_libraries(unit_mavlink_log cc_systemlib)
endif()

ament_package()
