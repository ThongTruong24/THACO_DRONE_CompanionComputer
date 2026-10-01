find_package(GTest REQUIRED)

add_executable(test_cc_agent
    test_config_engine.cpp
    test_rollback_guard.cpp
    test_metrics_and_serial.cpp
    test_hardware_registry.cpp
    ../src/app_paths.cpp
    ../src/config_engine.cpp
    ../src/rollback_guard.cpp
    ../src/system_metrics.cpp
    ../src/hardware_registry.cpp
)

target_include_directories(test_cc_agent PRIVATE
    ../include
)

target_link_libraries(test_cc_agent PRIVATE
    GTest::gtest
    GTest::gtest_main
    pthread
)

include(GoogleTest)
gtest_discover_tests(test_cc_agent)
