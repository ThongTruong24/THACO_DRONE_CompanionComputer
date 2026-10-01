find_package(GTest REQUIRED)

add_executable(test_camera_stream
    test_camera_config.cpp
    ../src/config_manager.cpp
    ../src/neon_processor.cpp
)

target_include_directories(test_camera_stream PRIVATE
    ../include
    ${YAML_INCLUDE_DIRS}
)

target_link_libraries(test_camera_stream PRIVATE
    ${YAML_LIBRARIES}
    GTest::gtest
    GTest::gtest_main
    pthread
)

include(GoogleTest)
gtest_discover_tests(test_camera_stream)
