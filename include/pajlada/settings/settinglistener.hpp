#pragma once

#include <pajlada/signals/connection.hpp>

namespace pajlada {

namespace detail {

struct CallbackCaller {
    std::function<void()> cb;

    void
    CallCallback()
    {
        this->cb();
    }
};

}  // namespace detail

class SettingListener
{
public:
    using Callback = std::function<void()>;

    SettingListener()
    {
        this->callbackCaller = std::make_unique<detail::CallbackCaller>();
    }

    SettingListener(Callback cb)
    {
        this->callbackCaller = std::make_unique<detail::CallbackCaller>();

        this->callbackCaller->cb = cb;
    }

    void
    setCB(Callback cb)
    {
        this->callbackCaller->cb = cb;
    }

    SettingListener(SettingListener &&other) noexcept
    {
        this->callbackCaller = std::move(other.callbackCaller);

        this->managedConnections.swap(other.managedConnections);
    }

    SettingListener &
    operator=(SettingListener &&other) noexcept
    {
        this->callbackCaller = std::move(other.callbackCaller);
        this->managedConnections.swap(other.managedConnections);

        return *this;
    }

    SettingListener(const SettingListener &) = delete;
    SettingListener &operator=(const SettingListener &) = delete;

    template <typename AnySetting>
    void
    addSetting(AnySetting &setting, bool autoInvoke = false)
    {
        setting.connectSimple(std::bind(&detail::CallbackCaller::CallCallback,
                                        this->callbackCaller.get()),
                              this->managedConnections, autoInvoke);
    }

private:
    std::vector<Signals::ScopedConnection> managedConnections;

    std::unique_ptr<detail::CallbackCaller> callbackCaller{};
};

}  // namespace pajlada
