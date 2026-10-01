find_package(GTest REQUIRED)

# 1. IPC Integration test (UdsHubServer Hardware <-> RouterSupervisor)
add_executable(test_integration_ipc
    test_ipc_hardware_router.cpp
    ../src/cc-agent/src/hardware_registry.cpp
    ../src/cc-agent/src/uds_hub_server.cpp
    ../src/mavlink-router-controller/src/router_supervisor.cpp
)

target_include_directories(test_integration_ipc PRIVATE
    ../src/cc-agent/include
    ../src/mavlink-router-controller/include
)

target_link_libraries(test_integration_ipc PRIVATE
    GTest::gtest
    GTest::gtest_main
    pthread
)

# 2. MAVLink Loopback Integration test
add_executable(test_integration_mavlink
    test_mavlink_loopback.cpp
)

target_include_directories(test_integration_mavlink PRIVATE
    ../src/mavros/mavlink/agridrone-thaco-mavlink/generated/c_library
    ../src/mavros/mavlink/agridrone-thaco-mavlink/generated/c_library/thaco_common
)

target_link_libraries(test_integration_mavlink PRIVATE
    GTest::gtest
    GTest::gtest_main
    pthread
)

include(GoogleTest)
gtest_discover_tests(test_integration_ipc)
gtest_discover_tests(test_integration_mavlink)
