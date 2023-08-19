#pragma once

#include <memory>
#include <mutex>
#include <pajlada/signals/scoped-connection.hpp>
#include <utility>
#include <vector>

namespace pajlada {

class SettingListener
{
    std::mutex cbMutex;
    std::function<void()> cb;

public:
    using Callback = std::function<void()>;

    SettingListener() = default;

    ~SettingListener()
    {
        this->resetCB();
    }

    SettingListener(Callback callback)
        : cb(std::move(callback))
    {
    }

    void
    setCB(Callback callback)
    {
        std::unique_lock<std::mutex> lock(this->cbMutex);
        this->cb = std::move(callback);
    }

    void
    resetCB()
    {
        std::unique_lock<std::mutex> lock(this->cbMutex);
        this->cb = std::function<void()>();
    }

    SettingListener(SettingListener &&other) = delete;
    SettingListener &operator=(SettingListener &&other) = delete;
    SettingListener(const SettingListener &) = delete;
    SettingListener &operator=(const SettingListener &) = delete;

    // templated function that can take any sort of pajlada::Setting
    template <typename AnySetting>
    void
    addSetting(AnySetting &setting, bool autoInvoke = false)
    {
        setting.connectSimple(std::bind(&SettingListener::invoke, this),
                              this->managedConnections, autoInvoke);
    }

    void
    invoke()
    {
        std::unique_lock<std::mutex> lock(this->cbMutex);

        if (this->cb) {
            this->cb();
        }
    }

private:
    std::vector<std::unique_ptr<Signals::ScopedConnection>> managedConnections;
};

}  // namespace pajlada
