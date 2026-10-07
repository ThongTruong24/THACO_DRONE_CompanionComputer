find_package(ament_cmake_gtest REQUIRED)

ament_add_gtest(test_router_config_generator
    test_router_config_generator.cpp
)
target_link_libraries(test_router_config_generator
    mavlink_router_core
)

ament_add_gtest(test_router_stats_parser
    test_router_stats_parser.cpp
)
target_link_libraries(test_router_stats_parser
    mavlink_router_core
)

ament_add_gtest(test_serial_link_monitor
    test_serial_link_monitor.cpp
)
target_link_libraries(test_serial_link_monitor
    mavlink_router_core
)

ament_add_gtest(test_mavlink_router
    test_mavlink_router.cpp
)
target_link_libraries(test_mavlink_router
    mavlink_router_core
)

ament_add_gtest(test_mavlink_router_node
    test_mavlink_router_node.cpp
)
target_link_libraries(test_mavlink_router_node
    mavlink_router_core
)
