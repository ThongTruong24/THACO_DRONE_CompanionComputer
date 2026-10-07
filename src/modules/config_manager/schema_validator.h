#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

namespace cc
{

enum class ParamTier {
	TIER1_CRITICAL = 1,
	TIER2_BOUNDED = 2,
	TIER3_OPTIONAL = 3
};

struct SchemaParam {
	std::string key;        // e.g. "router.fmu_baud"
	std::string section;    // e.g. "router"
	std::string name;       // e.g. "fmu_baud"
	std::string mav_id;     // e.g. "CC_RTR_BAUD"
	std::string node;       // e.g. "config_manager"
	std::string ros_name;   // e.g. "router.fmu_baud"
	std::string type;       // "int", "float", "string", "bool"
	ParamTier tier{ParamTier::TIER2_BOUNDED};
	std::string description;

	std::optional<double> min_val;
	std::optional<double> max_val;
	std::vector<std::string> options;

	YAML::Node default_val;
};

struct ValidationResult {
	bool is_valid{true};
	bool is_degraded{false};
	std::vector<std::string> warnings;
	std::vector<std::string> errors;
	YAML::Node validated_config;
};

class SchemaValidator
{
public:
	SchemaValidator() = default;

	bool load_schema(const std::string &schema_file);
	bool is_schema_loaded() const { return !_params.empty(); }

	ValidationResult validate(const YAML::Node &user_config, const YAML::Node &default_config);
	ValidationResult validate_file(const std::string &user_file, const std::string &default_file);

	static nlohmann::json to_json_cache(const YAML::Node &node);
	static bool write_active_cache(const std::string &cache_path, const YAML::Node &validated_config);
	static bool save_config_atomic(const std::string &file_path, const YAML::Node &config);

	const std::unordered_map<std::string, SchemaParam> &get_params() const { return _params; }

private:
	std::unordered_map<std::string, SchemaParam> _params;
};

} // namespace cc
