# Changelog

## Unreleased

- Breaking: Remove move ctor/operator for SettingListener. (#32)
- Dev: Remove `using namespace std` usages. (#38)

## v0.1.0

- Dev: Update serialize library to v0.1.0. (#34)
- Dev: Update signals library. (#24)
- Breaking: `USE_BOOST_FILESYSTEM` option has been renamed to `PAJLADA_SETTINGS_USE_BOOST_FILESYSTEM`. (#23)
- Breaking: `USE_CONAN` option has been renamed to `PAJLADA_SETTINGS_USE_CONAN`. (#23)
- Bump testing Boost version from v1.71.0 to v1.75.0
- Respect symlinks in the saving process
- Handle absolute symlinks properly
