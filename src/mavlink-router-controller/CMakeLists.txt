cmake_minimum_required(VERSION 3.20)
project(mavlink-router-controller VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -O2 -pthread)

include_directories(include)

add_executable(mavlink-router-controller
    src/main.cpp
    src/router_supervisor.cpp
)

target_link_libraries(mavlink-router-controller PRIVATE pthread)

install(TARGETS mavlink-router-controller DESTINATION /usr/local/bin)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
