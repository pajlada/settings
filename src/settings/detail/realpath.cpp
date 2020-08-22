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
        return path;
    }

    if (!isSymlink) {
        return path;
    }

    const auto relativePath = path.relative_path();

    std::unordered_set<std::string> seenPaths;

    do {
        if (seenPaths.count(path) != 0) {
            ec = std::make_error_code(std::errc::too_many_symbolic_link_levels);
            PS_DEBUG("Too many symbolic links");
            PS_DEBUG(ec);
            return path;
        }

        seenPaths.emplace(path);
        path = relativePath / fs::read_symlink(path, ec);
        if (ec) {
            PS_DEBUG("Error reading symlink");
            PS_DEBUG(ec);
            return path;
        }
        isSymlink = fs::is_symlink(path, ec);
        if (ec) {
            PS_DEBUG("Error checking symlink status");
            PS_DEBUG(ec);
            return path;
        }

    } while (isSymlink);

    return path;
}

}  // namespace detail
}  // namespace Settings
}  // namespace pajlada
