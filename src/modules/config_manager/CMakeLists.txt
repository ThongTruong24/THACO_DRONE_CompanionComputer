cmake_minimum_required(VERSION 3.16)
project(config_manager)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cc_msgs REQUIRED)
find_package(cc_link_config REQUIRED)
find_package(yaml-cpp REQUIRED)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/cc_module.cmake)

set(SRCS
    load_mon.cpp
    link_store.cpp
    hotspot_store.cpp
    schema_validator.cpp
    config_manager.cpp
)

cc_add_module(
    MODULE config_manager
    SRCS ${SRCS}
    DEPENDS
        rclcpp::rclcpp
        ${cc_msgs_TARGETS}
        ${cc_link_config_LIBRARIES}
        yaml-cpp
)

target_include_directories(config_manager PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/hrt/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/systemlib/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/link_config/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/atomic_file/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/nlohmann_json/include>
    $<INSTALL_INTERFACE:include>
)

add_executable(config_manager_node config_manager_main.cpp)
target_link_libraries(config_manager_node config_manager yaml-cpp)

if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)

    cc_add_unit_gtest(SRC tests/test_arming_latch.cpp LINKLIBS config_manager)
    cc_add_unit_gtest(SRC tests/test_load_mon.cpp LINKLIBS config_manager)
    cc_add_unit_gtest(SRC tests/test_link_store.cpp LINKLIBS config_manager)
    cc_add_unit_gtest(SRC tests/test_hotspot_store.cpp LINKLIBS config_manager)
    cc_add_unit_gtest(SRC tests/test_schema_validator.cpp LINKLIBS config_manager yaml-cpp)
    cc_add_unit_gtest(SRC tests/test_config_manager.cpp LINKLIBS config_manager)
endif()

install(TARGETS config_manager config_manager_node
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    config_manager.h
    link_store.h
    hotspot_store.h
    load_mon.h
    arming_latch.h
    schema_validator.h
    DESTINATION include
)

install(DIRECTORY config/
    DESTINATION share/${PROJECT_NAME}/config
)

ament_export_include_directories(include)
ament_export_libraries(config_manager)
ament_package()
