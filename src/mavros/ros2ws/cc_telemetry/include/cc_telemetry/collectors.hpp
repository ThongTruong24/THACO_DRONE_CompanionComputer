#pragma once
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <string>
namespace cc {
struct Snapshot { nlohmann::json values; std::string diagnostic; };
class Collectors {
public:
 explicit Collectors(YAML::Node config):config_(std::move(config)) {}
 Snapshot collect(const std::string & group,bool connected) const;
private:
 YAML::Node config_;
};
}
