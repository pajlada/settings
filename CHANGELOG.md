# Changelog

## Unreleased

- Breaking: Removed `SettingManager`s `pp`, `gPP`, and `stringify` functions. (#153)
- Breaking: `SettingManager::removeSetting` is no longer static. A static replacement is available as `SettingManager::gRemoveSetting`. Avoid using `Setting::remove()` if possible. (#151)
- Minor: Added a load option to `SettingManager` that allows for taking the `.tmp` file into consideration when loading a settings file. (#155)
- Dev: Fixed code coverage not generating for source files. (#149)
- Dev: Clean up the SaveSymlink test. (#145)
- Dev: Add instanced overloads for `arraySize` and `getObjectKeys`. (#148)
- Dev: Add instanced overloads for `Setting::set` and `Setting::get`. (#150)
- Dev: Use `std::string` over `const char *` where possible (e.g. in RapidJSON calls or fstreams). (#132)
- Dev: Refactored how we resolve paths, relying more on `std::filesystem` instead of rolling our own code. (#154)

## v0.3.0

- Breaking: Save methods now return a `SaveResult` enum instead of a bool. (#105)
- Minor: Setting managers can be configured to only save if a change has come in through its `set` function (e.g. by changing the value of a setting) with the `SaveMethod::OnlySaveIfChanged` flag. (#105)

## v0.2.2

- Bugfix: Renamed `slots` in the backup options to `numSlots` to avoid conflicts with Qt (#92)

## v0.2.1

- Minor: Added standalone backup API `pajlada::Settings::Backup::saveWithBackup` in `backup.hpp` (#90)

## v0.2.0

- Breaking: Remove move ctor/operator for SettingListener. (#32)
- Breaking: Updated to C++20. (#42)
- Breaking: Remove boost::any `userData` support. (#44)
- Breaking: Remove support for `boost::any`. (#46)
- Breaking: Remove support for `boost::filesystem`. (#47)
- Breaking: Remove support for `boost::optional`. (#48)
- Major: On Windows, use platform-specific [MOVEFILE_WRITE_THROUGH](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw#movefile_write_through) flag ensuring the move takes place before the function returns. (#87)
- Minor: Added setting option `CompareBeforeSet` which compares the old & new value in `setValue` before trying to update the value. This compares the marshalled JSON blob the value makes. (#74)
- Minor: Added support for `std::any`. (#46)
- Bugfix: Fixed an issue where settings without a value would always try to unmarshal the internal JSON. (#40)
- Dev: Remove `using namespace std` usages. (#38)

## v0.1.0

- Dev: Update serialize library to v0.1.0. (#34)
- Dev: Update signals library. (#24)
- Breaking: `USE_BOOST_FILESYSTEM` option has been renamed to `PAJLADA_SETTINGS_USE_BOOST_FILESYSTEM`. (#23)
- Breaking: `USE_CONAN` option has been renamed to `PAJLADA_SETTINGS_USE_CONAN`. (#23)
- Bump testing Boost version from v1.71.0 to v1.75.0
- Respect symlinks in the saving process
- Handle absolute symlinks properly
