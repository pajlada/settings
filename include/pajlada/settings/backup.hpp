#pragma once

#include <filesystem>
#include <functional>
#include <system_error>

namespace pajlada::Settings::Backup {

struct Options {
    /// Whether the backup is done or not
    bool enabled = true;
    /// The number of backup files to use
    std::uint8_t slots = 3;
};

/// @brief Orchestrates saving of a file (`path`) with optional backups
///
/// The save operation works as follows:
/// 1. The desired contents are written to a temporary file by `doWrite`
/// 2. Past backups are shifted by one (`.bkp-1` -> `.bkp-2`; `.bkp-2` ->
///    `.bkp-3` etc.)
/// 3. The current file (`path` - not the temporary one) is moved to the backup
///    (`.bkp-1`)
/// 4. The temporary file from step 1 is moved to the current file (`path`)
///
/// The procedure is successful if and only if the value of `ec` is `0` after
/// calling this function.
///
/// @param path The file path to save to ("current file")
/// @param options Options for the backup procedure (e.g. number of backup
///                files)
/// @param doWrite A callback to perform the write of the actual data. The
///                callback _must_ use the path passed to the callback instead
///                of the one passed to this function.
/// @param ec An error code to bubble up errors from operations done in this
///           procedure.
/// @returns Nothing - the caller must check `ec`.
void saveWithBackup(const std::filesystem::path &path, Options options,
                    const std::function<void(const std::filesystem::path &,
                                             std::error_code &ec)> &doWrite,
                    std::error_code &ec);

}  // namespace pajlada::Settings::Backup
