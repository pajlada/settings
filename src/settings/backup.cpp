#include <pajlada/settings/backup.hpp>
#include <pajlada/settings/detail/realpath.hpp>
#include <pajlada/settings/detail/rename.hpp>

namespace pajlada::Settings::Backup {

void
saveWithBackup(const std::filesystem::path &path, Options options,
               const std::function<void(const std::filesystem::path &,
                                        std::error_code &ec)> &doWrite,
               std::error_code &ec)
{
    std::filesystem::path realPath = detail::RealPath(path, ec);
    if (ec) {
        return;
    }
    std::filesystem::path tmpPath(path);
    tmpPath += ".tmp";

    std::filesystem::path bkpPath(path);
    bkpPath += ".bkp";

    doWrite(tmpPath, ec);
    if (ec) {
        return;
    }

    if (options.enabled) {
        std::filesystem::path firstBkpPath(bkpPath);
        firstBkpPath += "-" + std::to_string(1);

        if (options.numSlots > 1) {
            std::filesystem::path topBkpPath(bkpPath);
            topBkpPath += "-" + std::to_string(options.numSlots);
            topBkpPath = detail::RealPath(topBkpPath, ec);
            if (ec) {
                return;
            }
            // Remove top slot backup
            std::filesystem::remove(topBkpPath, ec);

            // Shift backups one slot up
            for (uint8_t slotIndex = options.numSlots - 1; slotIndex >= 1;
                 --slotIndex) {
                std::filesystem::path p1(bkpPath);
                p1 += "-" + std::to_string(slotIndex);
                p1 = detail::RealPath(p1, ec);
                if (ec) {
                    return;
                }
                std::filesystem::path p2(bkpPath);
                p2 += "-" + std::to_string(slotIndex + 1);
                p2 = detail::RealPath(p2, ec);
                if (ec) {
                    return;
                }
                detail::renameFile(p1, p2, ec);
            }
        }

        // Move current save to first backup slot
        detail::renameFile(realPath, firstBkpPath, ec);
    }

    detail::renameFile(tmpPath, realPath, ec);
}

}  // namespace pajlada::Settings::Backup
