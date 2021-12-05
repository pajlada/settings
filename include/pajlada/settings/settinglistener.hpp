#pragma once

#include <pajlada/signals/scoped-connection.hpp>

#include <mutex>

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

    SettingListener(Callback cb)
    {
        std::unique_lock<std::mutex> lock(this->cbMutex);
        this->cb = cb;
    }

    void
    setCB(Callback cb)
    {
        std::unique_lock<std::mutex> lock(this->cbMutex);
        this->cb = cb;
    }

    void
    resetCB()
    {
        std::unique_lock<std::mutex> lock(this->cbMutex);
        this->cb = std::function<void()>();
    }

    SettingListener(SettingListener &&other) noexcept
    {
        this->cb = std::move(other.cb);

        this->managedConnections.swap(other.managedConnections);
    }

    SettingListener &
    operator=(SettingListener &&other) noexcept
    {
        this->cb = std::move(other.cb);

        this->managedConnections.swap(other.managedConnections);

        return *this;
    }

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
