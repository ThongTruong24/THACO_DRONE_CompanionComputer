find_package(GTest REQUIRED)

add_executable(test_mavlink_router
    test_config_generator.cpp
    test_router_supervisor.cpp
    ../src/router_supervisor.cpp
)

target_include_directories(test_mavlink_router PRIVATE
    ../include
)

target_link_libraries(test_mavlink_router PRIVATE
    GTest::gtest
    GTest::gtest_main
    pthread
)

include(GoogleTest)
gtest_discover_tests(test_mavlink_router)
