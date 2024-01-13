#pragma once

#include <filesystem>

namespace pajlada {
namespace Settings {
namespace detail {

std::filesystem::path RealPath(const std::filesystem::path &_path,
                               std::error_code &ec);

}  // namespace detail
}  // namespace Settings
}  // namespace pajlada
