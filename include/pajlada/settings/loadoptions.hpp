#pragma once

namespace pajlada::Settings {

struct LoadOptions {
    /// If enabled, when load is called and fails, we will try to load settings from the temporary path (e.g. settings.json.tmp)
    bool attemptLoadFromTemporaryFile = false;
};

}  // namespace pajlada::Settings
