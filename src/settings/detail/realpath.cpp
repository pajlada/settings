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
        ec = {};
        return path;
    }

    if (!isSymlink) {
        return path;
    }

    const auto relativePath = path.parent_path();

    PS_DEBUG("Relative path: " << relativePath);

    std::unordered_set<fs::path::string_type> seenPaths;

    do {
        auto pathString = path.string();
        if (seenPaths.count(pathString) != 0) {
            ec = make_error_code_ns::make_error_code(
                errc::too_many_symbolic_link_levels);
            PS_DEBUG("Too many symbolic links");
            PS_DEBUG(ec);
            return path;
        }

        seenPaths.emplace(pathString);
        path = relativePath / fs::read_symlink(path, ec);
        if (ec) {
            PS_DEBUG("Error reading symlink");
            PS_DEBUG(ec);
            return path;
        }
        isSymlink = fs::is_symlink(path, ec);
        if (ec) {
            // Not an error - the symlink might have just stopped here at a file that doesn't exist (yet)
            ec = {};
            return path;
        }

    } while (isSymlink);

    return path;
}

}  // namespace detail
}  // namespace Settings
}  // namespace pajlada
