#pragma once

#include <pajlada/settings/setting.hpp>
#include <pajlada/settings/settingdata.hpp>

#include <pajlada/signals/connection.hpp>

namespace pajlada {
namespace Settings {

class SettingListener
{
public:
    using Callback = std::function<void(const SignalArgs &)>;

    SettingListener() = default;

    SettingListener(Callback _cb)
        : cb(std::move(_cb))
    {
    }

    SettingListener(SettingListener &&other) noexcept
        : cb(std::move(other.cb))
        , settings(std::move(other.settings))
    {
        other.managedConnections.clear();

        for (auto &setting : this->settings) {
            if (auto s = setting.lock()) {
                this->addRawSetting(s);
            }
        }
    }

    SettingListener &
    operator=(SettingListener &&other) noexcept
    {
        other.managedConnections.clear();

        this->cb = std::move(other.cb);
        this->settings = std::move(other.settings);

        for (auto &setting : this->settings) {
            if (auto s = setting.lock()) {
                this->addRawSetting(s);
            }
        }

        return *this;
    }

    SettingListener(const SettingListener &) = delete;
    SettingListener &operator=(const SettingListener &) = delete;

    void
    addRawSetting(std::shared_ptr<SettingData> setting)
    {
        auto connection = setting->updated.connect(
            [this](const rapidjson::Value &, const SignalArgs &args) {
                if (this->cb) {
                    this->cb(args);
                }
            });

        this->managedConnections.emplace_back(std::move(connection));
    }

    template <typename AnySetting>
    void
    addSetting(AnySetting &setting, bool autoInvoke = false)
    {
        this->settings.emplace_back(setting.getData());

        setting.connectSimple(
            [this](const SignalArgs &args) {
                if (this->cb) {
                    this->cb(args);
                }
            },
            this->managedConnections, autoInvoke);
    }

    Callback cb;

private:
    std::vector<std::weak_ptr<SettingData>> settings;

    std::vector<Signals::ScopedConnection> managedConnections;
};

}  // namespace Settings
}  // namespace pajlada
