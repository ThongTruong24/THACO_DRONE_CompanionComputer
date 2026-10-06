cmake_minimum_required(VERSION 3.16)
project(cc_mavlink)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cc_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/cc_module.cmake)

set(SRCS
    mavlink_main.cpp
    mavlink_stream.cpp
    mavlink_messages.cpp
    mavlink_receiver.cpp
    mavlink_parameters.cpp
)

cc_add_module(
    MODULE cc_mavlink
    SRCS ${SRCS}
    DEPENDS
        rclcpp::rclcpp
        ${cc_msgs_TARGETS}
        yaml-cpp
)

target_include_directories(cc_mavlink PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/hrt/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/mavlink>
    $<INSTALL_INTERFACE:include>
)

if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)

    cc_add_unit_gtest(SRC tests/test_mavlink_stream.cpp LINKLIBS cc_mavlink)
    cc_add_unit_gtest(SRC tests/test_streams.cpp LINKLIBS cc_mavlink)
    cc_add_unit_gtest(SRC tests/test_mavlink_receiver.cpp LINKLIBS cc_mavlink)
    cc_add_unit_gtest(SRC tests/test_mavlink_socket.cpp LINKLIBS cc_mavlink)
    cc_add_unit_gtest(SRC tests/test_mavlink_parameters.cpp LINKLIBS cc_mavlink yaml-cpp)
endif()

install(TARGETS cc_mavlink
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    mavlink_main.h
    mavlink_stream.h
    mavlink_messages.h
    mavlink_receiver.h
    mavlink_parameters.h
    mavlink_bridge_header.h
    DESTINATION include
)

ament_export_include_directories(include)
ament_export_libraries(cc_mavlink)
ament_package()
