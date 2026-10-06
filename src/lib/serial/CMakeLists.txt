cmake_minimum_required(VERSION 3.20)
project(cc_serial)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)

add_library(cc_serial STATIC
    src/hardware_registry.cpp
)

target_include_directories(cc_serial PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)
    ament_add_gtest(unit_serial_hardware_registry tests/test_hardware_registry.cpp)
    target_link_libraries(unit_serial_hardware_registry cc_serial)
endif()

install(TARGETS cc_serial
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

ament_export_include_directories(include)
ament_export_libraries(cc_serial)
ament_package()
