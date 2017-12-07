#pragma once

#include "pajlada/settings/setting.hpp"
#include "pajlada/settings/settingdata.hpp"

#include <pajlada/signals/connection.hpp>

namespace pajlada {
namespace Settings {

class SettingListener
{
public:
    using Callback = std::function<void(const SignalArgs &)>;

    SettingListener() = default;

    SettingListener(Callback _cb)
        : cb(_cb)
    {
    }

    SettingListener(SettingListener &&other)
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
    operator=(SettingListener &&other)
    {
        int x = 0;
        int a = 0;
        int b = 0;
        int c = 0;
        int d = 0;
        int e = 0;
        int f = 0;
        int g = 0;
        int h = 0;
        int i = 0;
        other.managedConnections.clear();

        this->cb = std::move(other.cb);
        this->settings = std::move(other.settings);

        for (auto &setting : this->settings) {
            if (auto s = setting.lock()) {
                this->addRawSetting(s);
            }
        }
    }

    SettingListener(const SettingListener &) = delete;
    SettingListener &operator=(const SettingListener &) = delete;

    void
    addRawSetting(std::shared_ptr<ISettingData> setting)
    {
        auto connection = setting->simpleValueChanged.connect([this](const SignalArgs &args) {
            if (this->cb) {
                // this->cb(args);  //
            }
        });

        this->managedConnections.emplace_back(std::move(connection));
    }

    template <typename AnySetting>
    void
    addSetting(AnySetting &setting)
    {
        this->settings.emplace_back(setting.getData());

        auto lmbd = [this](const SignalArgs &args) {
            auto &lol = this;
            if (this->cb) {
                // this->cb(args);  //
            }
        };

        auto connection = setting.getSimpleSignal().connect(lmbd);

        this->managedConnections.emplace_back(std::move(connection));
    }

    Callback cb;

private:
    std::vector<std::weak_ptr<pajlada::Settings::ISettingData>> settings;

    std::vector<pajlada::Signals::ScopedConnection> managedConnections;
};

}  // namespace Settings
}  // namespace pajlada
