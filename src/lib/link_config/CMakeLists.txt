cmake_minimum_required(VERSION 3.20)
project(cc_link_config)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)

add_library(cc_link_config STATIC
    src/link_config.cpp
)

target_include_directories(cc_link_config PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../atomic_file/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../nlohmann_json/include>
    $<INSTALL_INTERFACE:include>
)

if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)
    ament_add_gtest(unit_link_config tests/test_link_config.cpp)
    target_link_libraries(unit_link_config cc_link_config)
endif()

install(TARGETS cc_link_config
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

ament_export_include_directories(include)
ament_export_libraries(cc_link_config)
ament_package()
