#pragma once

#include <cstdio>
#include <fcntl.h>
#include <string>
#include <unistd.h>

namespace cc
{

// temp + fsync + rename in the same directory: a power cut leaves the old or the new file,
// never a truncated one. Bind-mount the DIRECTORY (renaming over a single-file bind mount fails with EBUSY).
inline bool write_file_atomic(const std::string &path, const std::string &content)
{
	const std::string tmp = path + ".tmp";
	int fd = ::open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);

	if (fd < 0) { return false; }

	size_t off = 0;

	while (off < content.size()) {
		ssize_t n = ::write(fd, content.data() + off, content.size() - off);

		if (n <= 0) {
			::close(fd);
			std::remove(tmp.c_str());
			return false;
		}

		off += static_cast<size_t>(n);
	}

	const bool ok = ::fsync(fd) == 0;

	if (::close(fd) != 0 || !ok || std::rename(tmp.c_str(), path.c_str()) != 0) {
		std::remove(tmp.c_str());
		return false;
	}

	const auto slash = path.find_last_of('/');
	int dfd = ::open(slash == std::string::npos ? "." : path.substr(0, slash ? slash : 1).c_str(), O_RDONLY | O_DIRECTORY);

	if (dfd >= 0) {
		::fsync(dfd);
		::close(dfd);
	}

	return true;
}

} // namespace cc
