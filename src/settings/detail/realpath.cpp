#include <pajlada/settings/detail/realpath.hpp>
#include <pajlada/settings/internal.hpp>
#include <unordered_set>

namespace pajlada::Settings::detail {

std::filesystem::path
RealPath(const std::filesystem::path &_path, std::error_code &ec)
{
    std::filesystem::path path{_path};

    auto isSymlink = std::filesystem::is_symlink(path, ec);

    if (ec) {
        // Not an error - the symlink might have just stopped here at a file that doesn't exist (yet)
        ec = {};
        return path;
    }

    if (!isSymlink) {
        return path;
    }

    const auto relativePath = path.parent_path();

    std::unordered_set<std::filesystem::path::string_type> seenPaths;

    do {
        auto pathString = path.native();
        if (seenPaths.count(pathString) != 0) {
            ec = std::make_error_code(std::errc::too_many_symbolic_link_levels);
            return path;
        }

        seenPaths.emplace(pathString);
        auto symlinkResponse = std::filesystem::read_symlink(path, ec);
        if (!symlinkResponse.is_absolute()) {
            path = relativePath / symlinkResponse;
        } else {
            path = symlinkResponse;
        }
        if (ec) {
            return path;
        }
        isSymlink = std::filesystem::is_symlink(path, ec);
        if (ec) {
            // Not an error - the symlink might have just stopped here at a file that doesn't exist (yet)
            ec = {};
            return path;
        }

    } while (isSymlink);

    return path;
}

}  // namespace pajlada::Settings::detail
