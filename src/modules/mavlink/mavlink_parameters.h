#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <cstring>
#include <rclcpp/rclcpp.hpp>

class Mavlink;
struct __mavlink_message;
typedef struct __mavlink_message mavlink_message_t;

enum class ParamType {
	UINT8,   // bool / uint8
	UINT32,
	INT32,
	REAL32,
	STRING   // custom string
};

struct ParamValue {
	ParamType type{ParamType::REAL32};
	uint32_t u32{0};
	int32_t i32{0};
	float r32{0.0f};
	uint8_t u8{0};
	std::string s;

	static ParamValue U8(uint8_t v)     { ParamValue p; p.type = ParamType::UINT8; p.u8 = v; return p; }
	static ParamValue U32(uint32_t v)   { ParamValue p; p.type = ParamType::UINT32; p.u32 = v; return p; }
	static ParamValue I32(int32_t v)    { ParamValue p; p.type = ParamType::INT32; p.i32 = v; return p; }
	static ParamValue R32(float v)      { ParamValue p; p.type = ParamType::REAL32; p.r32 = v; return p; }
	static ParamValue Str(std::string v) { ParamValue p; p.type = ParamType::STRING; p.s = std::move(v); return p; }

	bool operator==(const ParamValue &o) const
	{
		if (type != o.type) { return false; }

		switch (type) {
		case ParamType::UINT8:  return u8 == o.u8;

		case ParamType::UINT32: return u32 == o.u32;

		case ParamType::INT32:  return i32 == o.i32;

		case ParamType::REAL32: return r32 == o.r32;

		case ParamType::STRING: return s == o.s;
		}

		return false;
	}
};

struct ParamEntry {
	std::string id;         // <= 16 chars
	std::string node_name;  // e.g. "config_manager", "camera_streamer", "vision"
	std::string ros_name;   // e.g. "link0.port"
	ParamType type{ParamType::STRING};
	ParamValue value;
};

class MavlinkParametersManager
{
public:
	using NodeReadyFn = std::function<bool(const std::string &node_name)>;
	using SetParamFn = std::function<bool(const std::string &node_name, const std::string &param_name, const ParamValue &val)>;

	explicit MavlinkParametersManager(Mavlink *mavlink);
	~MavlinkParametersManager() = default;

	void init();

	void handle_request_list(const mavlink_message_t *msg);
	void handle_request_read(const mavlink_message_t *msg);
	void handle_set(const mavlink_message_t *msg);

	void send_param_ext_value(uint16_t index, uint8_t target_sys = 0, uint8_t target_comp = 0);

	// Schema loading
	bool load_schema(const std::string &schema_file);

	// Direct access for testing and inspection
	size_t count() const; // active params count
	bool at(size_t active_index, ParamEntry &out) const;
	bool get(const std::string &id, ParamValue &out) const;
	bool set(const std::string &id, const ParamValue &val);

	void set_node_ready_checker(NodeReadyFn fn) { _node_ready_fn = std::move(fn); }
	void set_param_setter(SetParamFn fn) { _set_param_fn = std::move(fn); }

	const std::vector<ParamEntry> &all_params() const { return _params; }

private:
	void define_all_parameters();
	bool is_node_ready(const std::string &node_name) const;
	std::vector<size_t> get_active_indices() const;

	Mavlink *_mavlink{nullptr};
	std::vector<ParamEntry> _params; // sorted by id
	std::unordered_map<std::string, size_t> _id_to_index;

	NodeReadyFn _node_ready_fn;
	SetParamFn _set_param_fn;

	std::unordered_map<std::string, std::shared_ptr<rclcpp::AsyncParametersClient>> _clients;
};
