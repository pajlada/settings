#pragma once

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

#include <atomic>
#include <functional>
#include <memory>
#include <pajlada/serialize.hpp>
#include <pajlada/settings/common.hpp>
#include <pajlada/settings/equal.hpp>
#include <pajlada/settings/internal.hpp>
#include <pajlada/settings/settingmanager.hpp>
#include <pajlada/settings/signalargs.hpp>
#include <pajlada/signals/signal.hpp>
#include <string>
#include <vector>

namespace pajlada::Settings {

class SettingData
{
    SettingData(std::string _path, std::weak_ptr<SettingManager> _instance);

    // Setting path (i.e. /a/b/c/3/d/e)
    const std::string path;

    std::weak_ptr<SettingManager> instance;

    std::atomic<int> updateIteration{};

public:
    Signals::Signal<const rapidjson::Value &, const SignalArgs &> updated;

    const std::string &getPath() const;

    void notifyUpdate(const rapidjson::Value &value, SignalArgs args);

    bool
    marshalJSON(const rapidjson::Value &v)
    {
        auto locked = this->instance.lock();
        if (!locked) {
            return false;
        }

        return locked->set(this->path.c_str(), v);
    }

    template <typename Type>
    bool
    marshal(const Type &v, SignalArgs args = SignalArgs())
    {
        auto locked = this->instance.lock();
        if (!locked) {
            return false;
        }

        auto jsonValue =
            Serialize<Type>::get(v, locked->document.GetAllocator());

        return locked->set(this->path.c_str(), jsonValue, std::move(args));
    }

    rapidjson::Value *
    unmarshalJSON()
    {
        return this->get();
    }

    template <typename Type>
    std::optional<Type>
    unmarshal() const
    {
        auto *ptr = this->get();

        if (ptr == nullptr) {
            return std::nullopt;
        }

        return Deserialize<Type>::get(*ptr);
    }

    int getUpdateIteration() const;

private:
    friend class SettingManager;

    rapidjson::Value *get() const;
};

}  // namespace pajlada::Settings
