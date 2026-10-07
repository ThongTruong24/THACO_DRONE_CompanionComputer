find_package(ament_cmake_gtest REQUIRED)

ament_add_gtest(unit_ai_branch
    test_ai_branch.cpp
)
target_link_libraries(unit_ai_branch
    camera_streamer_core
)

ament_add_gtest(unit_camera_streamer_node
    test_camera_streamer_node.cpp
)
target_link_libraries(unit_camera_streamer_node
    camera_streamer_core
)
