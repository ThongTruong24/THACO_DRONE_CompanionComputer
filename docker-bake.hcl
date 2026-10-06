variable "REGISTRY" {
  default = ""
}

variable "TAG" {
  default = "latest"
}

function "tag" {
  params = [name]
  result = [
    "${REGISTRY}${name}:${TAG}"
  ]
}

group "default" {
  targets = ["base", "builder", "router", "agent", "network", "camera"]
}

group "build-all" {
  targets = ["base", "builder", "router", "agent", "network", "camera"]
}

target "base" {
  dockerfile = "platforms/linux/docker/base.Dockerfile"
  target = "base"
  tags = tag("edge-ros-base")
}

target "builder" {
  dockerfile = "platforms/linux/docker/base.Dockerfile"
  target = "builder"
  tags = tag("edge-ros-builder")
  contexts = {
    base = "target:base"
  }
}

target "router" {
  dockerfile = "src/modules/mavlink_router/Dockerfile"
  tags = tag("edge-router")
  contexts = {
    edge-ros-base = "target:base"
    edge-ros-builder = "target:builder"
  }
}

target "agent" {
  dockerfile = "platforms/linux/edge_agent/Dockerfile"
  tags = tag("edge-agent")
  contexts = {
    edge-ros-base = "target:base"
    edge-ros-builder = "target:builder"
  }
}

target "camera" {
  dockerfile = "src/modules/camera_streamer/Dockerfile"
  tags = tag("edge-camera")
  contexts = {
    edge-ros-base = "target:base"
    edge-ros-builder = "target:builder"
  }
}

target "network" {
  dockerfile = "src/modules/networking/Dockerfile"
  tags = tag("edge-network")
  contexts = {
    edge-ros-base = "target:base"
    edge-ros-builder = "target:builder"
  }
}

target "vision" {
  dockerfile = "src/modules/vision/Dockerfile"
  tags = tag("edge-vision")
  contexts = {
    edge-ros-base = "target:base"
    edge-ros-builder = "target:builder"
  }
}
