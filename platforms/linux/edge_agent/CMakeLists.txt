cmake_minimum_required(VERSION 3.16)
project(edge_agent)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cc_msgs REQUIRED)
find_package(cc_mavlink REQUIRED)
find_package(config_manager REQUIRED)
find_package(cc_systemlib REQUIRED)
find_package(cc_link_config REQUIRED)
find_package(yaml-cpp REQUIRED)

add_executable(edge_agent src/edge_agent_main.cpp)

target_include_directories(edge_agent PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/modules/config_manager>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/modules/mavlink>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/mavlink>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/hrt/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/systemlib/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/link_config/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/atomic_file/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/lib/nlohmann_json/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(edge_agent PRIVATE
    rclcpp::rclcpp
    ${config_manager_LIBRARIES}
    ${cc_mavlink_LIBRARIES}
    ${cc_msgs_TARGETS}
    ${cc_systemlib_LIBRARIES}
    ${cc_link_config_LIBRARIES}
    yaml-cpp
)

install(TARGETS edge_agent
    RUNTIME DESTINATION bin
)

ament_package()
