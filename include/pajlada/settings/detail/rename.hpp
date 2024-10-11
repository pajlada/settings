#pragma once

#include <filesystem>

namespace pajlada::Settings::detail {

void renameFile(const std::filesystem::path &from,
                const std::filesystem::path &to, std::error_code &ec);

}  // namespace pajlada::Settings::detail
