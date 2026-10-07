cmake_minimum_required(VERSION 3.20)
project(cc_msgs)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/VehicleCommand.msg"
  "msg/VehicleCommandAck.msg"
  "msg/MavlinkLog.msg"
  "msg/VehicleStatus.msg"
  "msg/SystemStatus.msg"
  "msg/NetworkStatus.msg"
  "msg/CameraStatus.msg"
  "msg/VisionStatus.msg"
  "msg/SerialLink.msg"
  "msg/SerialLinkStatus.msg"
  "msg/Link.msg"
  "msg/LinkConfig.msg"
  "msg/FrameReady.msg"
  "srv/ApplyLinks.srv"
)

ament_package()
