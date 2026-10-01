cmake_minimum_required(VERSION 3.8)
project(plugin_agridrone)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cc_interfaces REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(mavros REQUIRED)
find_package(libmavconn REQUIRED)
find_package(pluginlib REQUIRED)
find_package(std_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geographic_msgs REQUIRED)
find_package(mavros_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_eigen REQUIRED)

add_library(agridrone_plugins SHARED
  src/agridrone_plugin.cpp
)

target_include_directories(agridrone_plugins PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(agridrone_plugins
  nlohmann_json::nlohmann_json
  ${CMAKE_DL_LIBS}
)

ament_target_dependencies(agridrone_plugins
  rclcpp
  cc_interfaces
  mavros
  libmavconn
  pluginlib
  std_msgs
  diagnostic_msgs
  sensor_msgs
  geometry_msgs
  nav_msgs
  geographic_msgs
  mavros_msgs
  tf2_ros
  tf2_eigen
)

pluginlib_export_plugin_description_file(mavros agridrone_plugins.xml)

install(TARGETS agridrone_plugins
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)

install(FILES agridrone_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(agridrone_plugins)
ament_export_dependencies(
  rclcpp
  cc_interfaces
  mavros
  libmavconn
  pluginlib
  std_msgs
  diagnostic_msgs
  sensor_msgs
  geometry_msgs
  nav_msgs
  geographic_msgs
  mavros_msgs
  tf2_ros
  tf2_eigen
)

ament_package()
