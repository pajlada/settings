#include <pajlada/settings/detail/realpath.hpp>

namespace pajlada::Settings::detail {

std::filesystem::path
RealPath(const std::filesystem::path &_path, std::error_code &ec)
{
    auto path = std::filesystem::weakly_canonical(_path, ec);
    if (ec) {
        return _path;
    }

    auto isSymlink = std::filesystem::is_symlink(path, ec);

    if (ec) {
        // Not an error - the symlink might have just stopped here at a file that doesn't exist (yet)
        ec.clear();
        return path;
    }

    if (!isSymlink) {
        return path;
    }

    auto symlinkTarget = std::filesystem::read_symlink(path, ec);
    if (ec) {
        // Something went wrong reading the symlink, return original canonical path
        return path;
    }
    if (symlinkTarget.is_absolute()) {
        return symlinkTarget;
    }

    auto absolutePath =
        std::filesystem::absolute(path.parent_path() / symlinkTarget, ec);
    if (ec) {
        // Something went wrong making the final symlink target absolute, return original canonical path
        return path;
    }
    return absolutePath;
}

}  // namespace pajlada::Settings::detail
