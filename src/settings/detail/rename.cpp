#include <pajlada/settings/detail/rename.hpp>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace pajlada::Settings::detail {

void
renameFile(const std::filesystem::path &from, const std::filesystem::path &to,
           std::error_code &ec)
{
#ifdef _WIN32
    // MOVEFILE_WRITE_THROUGH to bypass the filesystem cache
    if (MoveFileExW(from.c_str(), to.c_str(),
                    MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING |
                        MOVEFILE_WRITE_THROUGH) == TRUE) {
        ec = {0, std::system_category()};
    } else {
        ec = {static_cast<int>(GetLastError()), std::system_category()};
    }
#else
    std::filesystem::rename(from, to, ec);
#endif
}

}  // namespace pajlada::Settings::detail
