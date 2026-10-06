#include "link_store.h"

#include <iostream>
#include <sys/stat.h>

namespace cc
{

bool default_is_char_device(const std::string &path)
{
	struct stat st {};
	return ::stat(path.c_str(), &st) == 0 && S_ISCHR(st.st_mode);
}

LinkStore::LinkStore(const LinkStorePaths &paths, PortCheck port_exists, RouterApplyFn router_apply)
	: _paths(paths), _port_exists(std::move(port_exists)), _router_apply(std::move(router_apply))
{
	if (auto a = load_links_file(_paths.active_file)) {
		_active = *a;
		return;
	}

	if (auto d = load_links_file(_paths.default_file)) {
		_active = *d;
		save_links_file(_paths.active_file, _active);
		return;
	}

	std::cerr << "[LinkStore] no valid links.json in active or default file — starting empty\n";
}

Links LinkStore::active() const
{
	std::lock_guard<std::mutex> l(_mu);
	return _active;
}

ApplyResult LinkStore::apply(const Links &links, std::string &err)
{
	err = validate_links(links);

	if (!err.empty()) {
		return ApplyResult::Rejected;
	}

	if (_port_exists) {
		for (const auto &l : links) {
			if (!l.port.empty() && !_port_exists(l.port)) {
				err = l.name + ": port not found " + l.port;
				return ApplyResult::Rejected;
			}
		}
	}

	const Links previous = active();

	if (!save_links_file(_paths.active_file, links)) {
		err = "cannot write " + _paths.active_file;
		return ApplyResult::Failed;
	}

	if (_router_apply && !_router_apply(links, err)) {
		save_links_file(_paths.active_file, previous);
		return ApplyResult::Failed;
	}

	{
		std::lock_guard<std::mutex> l(_mu);
		_active = links;
	}

	return ApplyResult::Ok;
}

bool LinkStore::save_default(std::string &err)
{
	if (save_links_file(_paths.default_file, active())) {
		return true;
	}

	err = "cannot write " + _paths.default_file;
	return false;
}

ApplyResult LinkStore::restore_default(std::string &err)
{
	auto d = load_links_file(_paths.default_file);

	if (!d) {
		err = "no valid default " + _paths.default_file;
		return ApplyResult::Failed;
	}

	return apply(*d, err);
}

} // namespace cc
