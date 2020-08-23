#include <pajlada/settings/detail/realpath.hpp>
#include <pajlada/settings/internal.hpp>
#include <unordered_set>

namespace pajlada {
namespace Settings {
namespace detail {

fs::path
RealPath(const fs::path &_path, fs_error_code &ec)
{
    fs::path path{_path};

    auto isSymlink = fs::is_symlink(path, ec);

    if (ec) {
        // Not an error - the symlink might have just stopped here at a file that doesn't exist (yet)
        ec = {};
        return path;
    }

    if (!isSymlink) {
        return path;
    }

    const auto relativePath = path.parent_path();

    PS_DEBUG("Parent path: " << relativePath);

    std::unordered_set<fs::path::string_type> seenPaths;

    do {
        auto pathString = path.native();
        if (seenPaths.count(pathString) != 0) {
            ec = make_error_code_ns::make_error_code(
                errc::too_many_symbolic_link_levels);
            return path;
        }

        seenPaths.emplace(pathString);
        auto symlinkResponse = fs::read_symlink(path, ec);
        if (!symlinkResponse.is_absolute()) {
            path = relativePath / symlinkResponse;
        } else {
            PS_DEBUG("Is absolute xd: " << symlinkResponse);
            path = symlinkResponse;
        }
        if (ec) {
            PS_DEBUG("EC: " << ec);
            return path;
        }
        isSymlink = fs::is_symlink(path, ec);
        if (ec) {
            PS_DEBUG("not a symlink: " << ec);
            PS_DEBUG("Final path2: " << path);
            // Not an error - the symlink might have just stopped here at a file that doesn't exist (yet)
            ec = {};
            return path;
        }

    } while (isSymlink);

    PS_DEBUG("Final path: " << path);
    return path;
}

}  // namespace detail
}  // namespace Settings
}  // namespace pajlada
