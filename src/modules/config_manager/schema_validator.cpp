#include "schema_validator.h"
#include <atomic_file.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace cc
{

bool SchemaValidator::load_schema(const std::string &schema_file)
{
	_params.clear();
	try {
		YAML::Node root = YAML::LoadFile(schema_file);
		if (!root["parameters"] || !root["parameters"].IsMap()) {
			return false;
		}

		for (const auto &it : root["parameters"]) {
			std::string key = it.first.as<std::string>();
			YAML::Node pnode = it.second;

			SchemaParam sp;
			sp.key = key;

			auto dot = key.find('.');
			if (dot != std::string::npos) {
				sp.section = key.substr(0, dot);
				sp.name = key.substr(dot + 1);
			} else {
				sp.section = "general";
				sp.name = key;
			}

			if (pnode["mav_id"]) { sp.mav_id = pnode["mav_id"].as<std::string>(); }
			if (pnode["node"]) { sp.node = pnode["node"].as<std::string>(); }
			if (pnode["ros_name"]) { sp.ros_name = pnode["ros_name"].as<std::string>(); }
			if (pnode["type"]) { sp.type = pnode["type"].as<std::string>(); }
			if (pnode["desc"]) { sp.description = pnode["desc"].as<std::string>(); }

			if (pnode["tier"]) {
				int t = pnode["tier"].as<int>();
				sp.tier = (t == 1) ? ParamTier::TIER1_CRITICAL :
				          (t == 2) ? ParamTier::TIER2_BOUNDED : ParamTier::TIER3_OPTIONAL;
			}

			if (pnode["min"]) { sp.min_val = pnode["min"].as<double>(); }
			if (pnode["max"]) { sp.max_val = pnode["max"].as<double>(); }

			if (pnode["options"] && pnode["options"].IsSequence()) {
				for (const auto &opt : pnode["options"]) {
					sp.options.push_back(opt.as<std::string>());
				}
			}

			if (pnode["default"]) {
				sp.default_val = pnode["default"];
			}

			_params[key] = sp;
		}
		return true;
	} catch (const std::exception &e) {
		std::cerr << "[SchemaValidator] Exception loading schema: " << e.what() << "\n";
		return false;
	}
}

ValidationResult SchemaValidator::validate_file(const std::string &user_file, const std::string &default_file)
{
	YAML::Node def_node;
	try {
		def_node = YAML::LoadFile(default_file);
	} catch (const std::exception &e) {
		ValidationResult err_res;
		err_res.is_valid = false;
		err_res.errors.push_back(std::string("Cannot load factory default file: ") + e.what());
		return err_res;
	}

	YAML::Node user_node;
	bool user_loaded = false;
	try {
		user_node = YAML::LoadFile(user_file);
		user_loaded = true;
	} catch (const std::exception &e) {
		ValidationResult fallback_res;
		fallback_res.is_valid = true;
		fallback_res.is_degraded = true;
		fallback_res.warnings.push_back(std::string("User config missing or invalid syntax, falling back to default: ") + e.what());
		fallback_res.validated_config = YAML::Clone(def_node);
		return fallback_res;
	}

	return validate(user_node, def_node);
}

ValidationResult SchemaValidator::validate(const YAML::Node &user_config, const YAML::Node &default_config)
{
	ValidationResult result;
	result.validated_config = YAML::Clone(default_config);

	for (const auto &[key, param] : _params) {
		const std::string &sec = param.section;
		const std::string &name = param.name;

		YAML::Node user_val;
		if (user_config[sec] && user_config[sec][name]) {
			user_val = user_config[sec][name];
		} else {
			result.warnings.push_back("Missing parameter '" + key + "', using default value.");
			continue;
		}

		// Ensure section exists in validated_config
		if (!result.validated_config[sec]) {
			result.validated_config[sec] = YAML::Node(YAML::NodeType::Map);
		}

		try {
			if (param.type == "int") {
				int val = user_val.as<int>();
				bool out_of_options = false;

				if (!param.options.empty()) {
					std::string str_val = std::to_string(val);
					if (std::find(param.options.begin(), param.options.end(), str_val) == param.options.end()) {
						out_of_options = true;
					}
				}

				if (out_of_options) {
					result.is_degraded = true;
					result.errors.push_back("Parameter '" + key + "' value " + std::to_string(val) + " not in allowed options, reverted to default.");
					// Revert to default
					if (default_config[sec] && default_config[sec][name]) {
						result.validated_config[sec][name] = default_config[sec][name].as<int>();
					} else {
						result.validated_config[sec][name] = param.default_val.as<int>();
					}
				} else {
					if (param.min_val && val < static_cast<int>(*param.min_val)) {
						result.is_degraded = true;
						result.warnings.push_back("Clamped '" + key + "' to min " + std::to_string(static_cast<int>(*param.min_val)));
						val = static_cast<int>(*param.min_val);
					}
					if (param.max_val && val > static_cast<int>(*param.max_val)) {
						result.is_degraded = true;
						result.warnings.push_back("Clamped '" + key + "' to max " + std::to_string(static_cast<int>(*param.max_val)));
						val = static_cast<int>(*param.max_val);
					}
					result.validated_config[sec][name] = val;
				}

			} else if (param.type == "float") {
				double val = user_val.as<double>();
				if (param.min_val && val < *param.min_val) {
					result.is_degraded = true;
					result.warnings.push_back("Clamped '" + key + "' to min " + std::to_string(*param.min_val));
					val = *param.min_val;
				}
				if (param.max_val && val > *param.max_val) {
					result.is_degraded = true;
					result.warnings.push_back("Clamped '" + key + "' to max " + std::to_string(*param.max_val));
					val = *param.max_val;
				}
				result.validated_config[sec][name] = val;

			} else if (param.type == "bool") {
				bool val = user_val.as<bool>();
				result.validated_config[sec][name] = val;

			} else if (param.type == "string") {
				std::string val = user_val.as<std::string>();
				result.validated_config[sec][name] = val;
			}
		} catch (const std::exception &e) {
			result.is_degraded = true;
			result.errors.push_back("Type mismatch on '" + key + "': " + e.what() + ", using default.");
			if (default_config[sec] && default_config[sec][name]) {
				result.validated_config[sec][name] = YAML::Clone(default_config[sec][name]);
			}
		}
	}

	return result;
}

static nlohmann::json yaml_node_to_json(const YAML::Node &node)
{
	switch (node.Type()) {
	case YAML::NodeType::Null:
		return nullptr;
	case YAML::NodeType::Scalar: {
		// Attempt numeric / bool conversions
		try {
			return node.as<bool>();
		} catch (...) {}
		try {
			return node.as<int64_t>();
		} catch (...) {}
		try {
			return node.as<double>();
		} catch (...) {}
		return node.as<std::string>();
	}
	case YAML::NodeType::Sequence: {
		nlohmann::json j = nlohmann::json::array();
		for (const auto &item : node) {
			j.push_back(yaml_node_to_json(item));
		}
		return j;
	}
	case YAML::NodeType::Map: {
		nlohmann::json j = nlohmann::json::object();
		for (const auto &it : node) {
			j[it.first.as<std::string>()] = yaml_node_to_json(it.second);
		}
		return j;
	}
	default:
		return nullptr;
	}
}

nlohmann::json SchemaValidator::to_json_cache(const YAML::Node &node)
{
	return yaml_node_to_json(node);
}

bool SchemaValidator::write_active_cache(const std::string &cache_path, const YAML::Node &validated_config)
{
	nlohmann::json j = to_json_cache(validated_config);
	std::string content = j.dump(2);
	return cc::write_file_atomic(cache_path, content);
}

bool SchemaValidator::save_config_atomic(const std::string &file_path, const YAML::Node &config)
{
	YAML::Emitter emitter;
	emitter << config;
	std::string content = emitter.c_str();
	content += "\n";
	return cc::write_file_atomic(file_path, content);
}

} // namespace cc
