cmake_minimum_required(VERSION 3.20)
project(cc_frame_pool)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

# C++ library
add_library(cc_frame_pool STATIC
    src/FramePoolWriter.cpp
)

target_include_directories(cc_frame_pool PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../hrt/include>
    $<INSTALL_INTERFACE:include>
)

# Python package
ament_python_install_package(frame_pool PACKAGE_DIR frame_pool)

if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)
    find_package(ament_cmake_pytest QUIET)

    ament_add_gtest(unit_frame_pool tests/test_frame_pool_cpp.cpp)
    target_link_libraries(unit_frame_pool cc_frame_pool)

    add_executable(frame_pool_writer_helper tests/frame_pool_writer_helper.cpp)
    target_link_libraries(frame_pool_writer_helper cc_frame_pool)

    if(ament_cmake_pytest_FOUND)
        ament_add_pytest_test(test_frame_pool_reader tests/test_frame_pool_reader.py
            ENV FRAME_POOL_HELPER_BIN=$<TARGET_FILE:frame_pool_writer_helper>
            PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endif()

install(TARGETS cc_frame_pool
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

ament_export_include_directories(include)
ament_export_libraries(cc_frame_pool)

ament_package()
