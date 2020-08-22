#pragma once

#include <pajlada/settings/detail/fs.hpp>

namespace pajlada {
namespace Settings {
namespace detail {

fs::path RealPath(const fs::path &_path, fs_error_code &ec);

}  // namespace detail
}  // namespace Settings
}  // namespace pajlada
