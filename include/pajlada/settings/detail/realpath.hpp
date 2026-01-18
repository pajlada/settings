#pragma once

#include <filesystem>
#include <system_error>

namespace pajlada::Settings::detail {

/// Return the canonical path of `_path` if possible.
/// If the canonical path ends on a symlink that points to a non-existent path, return that non-existent path.
std::filesystem::path RealPath(const std::filesystem::path &_path,
                               std::error_code &ec);

}  // namespace pajlada::Settings::detail
