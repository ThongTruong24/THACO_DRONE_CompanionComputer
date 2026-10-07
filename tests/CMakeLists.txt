cmake_minimum_required(VERSION 3.20)
project(cc_integration_tests VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Test-only package: plain CMake and colcon (ament_cmake) both work.
find_package(ament_cmake QUIET)

if(BUILD_TESTING)
enable_testing()
include(GoogleTest)
find_package(GTest REQUIRED)



# 2. MAVLink Loopback Integration test
add_executable(test_integration_mavlink
    test_mavlink_loopback.cpp
)

target_include_directories(test_integration_mavlink PRIVATE
    ../src/lib/mavlink
    ../src/lib/mavlink/thaco
)

target_link_libraries(test_integration_mavlink PRIVATE
    GTest::gtest
    GTest::gtest_main
    pthread
)

# 3. MAVLink telemetry wire-format roundtrip (CC_TELEMETRY_LINKS field mapping)
add_executable(test_integration_telemetry
    test_mavlink_telemetry_roundtrip.cpp
)

target_include_directories(test_integration_telemetry PRIVATE
    ../src/lib/mavlink
    ../src/lib/mavlink/thaco
)

target_link_libraries(test_integration_telemetry PRIVATE
    GTest::gtest
    GTest::gtest_main
    pthread
)

# 4. PARAM_EXT wire roundtrip — compiled against the AGENT's MAVLink dialect
#    (src/lib/mavlink), which is what the agent actually encodes with.
add_executable(test_integration_param
    test_param_ext_wire.cpp
)

target_include_directories(test_integration_param PRIVATE
    ../src/lib/mavlink
    ../src/lib/mavlink/thaco
)

target_link_libraries(test_integration_param PRIVATE
    GTest::gtest
    GTest::gtest_main
    pthread
)

find_package(rclcpp REQUIRED)
find_package(cc_msgs REQUIRED)

# 5. Contract tests for edge_agent (router UDP black-box + StartOrderTest)
add_executable(test_contract_edge_agent
    contract/test_edge_agent_contract.cpp
)

target_include_directories(test_contract_edge_agent PRIVATE
    ../src/lib/mavlink
    ../src/lib/mavlink/thaco
    ../src/lib/systemlib/include
    ../src/lib/hrt/include
)

target_link_libraries(test_contract_edge_agent
    GTest::gtest
    GTest::gtest_main
    pthread
)

ament_target_dependencies(test_contract_edge_agent
    rclcpp
    cc_msgs
)

gtest_discover_tests(test_integration_mavlink)
gtest_discover_tests(test_integration_telemetry)
gtest_discover_tests(test_integration_param)
gtest_discover_tests(test_contract_edge_agent)
endif()

if(ament_cmake_FOUND)
    ament_package()
endif()
