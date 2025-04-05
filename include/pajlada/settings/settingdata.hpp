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

    void notifyUpdate(const rapidjson::Value &value, const SignalArgs &args);

    bool marshalJSON(const rapidjson::Value &v, const SignalArgs &args = {});

    template <typename Type>
    bool
    marshal(const Type &v, const SignalArgs &args = {})
    {
        auto locked = this->instance.lock();
        if (!locked) {
            return false;
        }

        auto jsonValue =
            Serialize<Type>::get(v, locked->document.GetAllocator());

        return locked->set(this->path, jsonValue, args);
    }

    bool unmarshalJSON(rapidjson::Document &doc);

    template <typename Type>
    std::optional<Type>
    unmarshal() const
    {
        auto locked = this->instance.lock();
        if (!locked) {
            return std::nullopt;
        }

        return locked->get(this->path, [](auto *ptr) -> std::optional<Type> {
            if (ptr == nullptr) {
                return std::nullopt;
            }

            return Deserialize<Type>::get(*ptr);
        });
    }

    int getUpdateIteration() const;

private:
    friend class SettingManager;
};

}  // namespace pajlada::Settings
