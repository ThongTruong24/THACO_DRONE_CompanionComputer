#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include <link_config.hpp>

namespace cc
{

struct LinkStorePaths {
	std::string active_file = "/run/drone/links.json";
	std::string default_file = "config/links.json";
};

enum class ApplyResult {
	Ok,
	Rejected,
	Failed
};

using PortCheck = std::function<bool(const std::string &port)>;
using RouterApplyFn = std::function<bool(const Links &links, std::string &err)>;

bool default_is_char_device(const std::string &path);

class LinkStore
{
public:
	explicit LinkStore(const LinkStorePaths &paths,
			   PortCheck port_exists = default_is_char_device,
			   RouterApplyFn router_apply = nullptr);

	Links active() const;

	ApplyResult apply(const Links &links, std::string &err);
	bool save_default(std::string &err);
	ApplyResult restore_default(std::string &err);

	void set_router_apply_fn(RouterApplyFn fn) { _router_apply = std::move(fn); }
	void set_port_check_fn(PortCheck fn) { _port_exists = std::move(fn); }

	const LinkStorePaths &paths() const { return _paths; }

private:
	LinkStorePaths _paths;
	PortCheck _port_exists;
	RouterApplyFn _router_apply;
	mutable std::mutex _mu;
	Links _active;
};

} // namespace cc
