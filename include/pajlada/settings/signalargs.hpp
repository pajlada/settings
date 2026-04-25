#pragma once

#include <string>

namespace pajlada::Settings {

struct SignalArgs {
    SignalArgs() = default;

    enum Source {
        Unset,
        Setter,
        Unmarshal,
        OnConnect,
        External,
    } source = Source::Unset;

    std::string path;

    bool writeToFile{true};
    bool compareBeforeSet{false};

private:
    /// If set to true, the SettingManager instance will attempt to
    /// only remove the setting from the RapidJSON document instead of
    /// setting the value to its default value.
    bool resetToDefault{false};

    friend class SettingManager;

    template <typename Type>
    friend class Setting;
};

}  // namespace pajlada::Settings
