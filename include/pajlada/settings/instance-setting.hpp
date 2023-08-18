#pragma once

#include <rapidjson/document.h>

#include <any>
#include <iostream>
#include <pajlada/settings/common.hpp>
#include <pajlada/settings/equal.hpp>
#include <pajlada/settings/instance.hpp>
#include <pajlada/settings/settingdata.hpp>
#include <pajlada/signals.hpp>
#include <shared_mutex>
#include <type_traits>

namespace pajlada {
namespace Settings {

// A default value passed to a setting is only local to this specific instance of the setting
// it is never shared between other settings at the same path
template <typename Type>
class InstanceSetting
{
    const std::string path;
    const std::shared_ptr<Instance> instance;

public:
    InstanceSetting(std::shared_ptr<Instance> _instance, std::string_view _path,
                    Type _defaultValue = {})
        : path(_path)
        , instance(_instance)
        , defaultValue(std::move(_defaultValue))
    {
        if constexpr (true) {
            // TODO: If Type is not serializable, give a nice readable compile error here
        }

        this->value = this->instance->get<Type>(this->path);
    }

    ~InstanceSetting() = default;

    const Type &
    getValue() const
    {
        std::shared_lock lock(this->mutex);

        if (this->value) {
            return *this->value;
        }

        return this->defaultValue;
    }

    void
    setValue(const Type &newValue)
    {
        std::unique_lock lock(this->mutex);

        // Set our own in-memory representation of the value
        // TODO: Should we just rely on the value from instance's key-value store (data)?
        this->value = newValue;

        this->instance->set(this->path, newValue);
    }

    bool
    operator==(const Type &rhs) const
    {
        return this->getValue() == rhs;
    }

    bool
    operator!=(const Type &rhs) const
    {
        return this->getValue() != rhs;
    }

    operator Type() const
    {
        return this->getValue();
    }

    // For testing
    Type
    _getDefaultValue() const
    {
        return this->defaultValue;
    }

private:
    const Type defaultValue{};

    mutable std::shared_mutex mutex;
    OptionalType<Type> value;

public:
    // Connect: Value
    void
    connect(std::function<void(const Type &)> func, bool autoInvoke = true)
    {
        auto connection = this->instance->registerOnValueUpdated(
            this->path, [func](const std::any &newValue) {
                // std::cout << newValue << "\n";
                std::cout << newValue.type().name() << "\n";
                auto actualValue = std::any_cast<Type>(newValue);
                // TODO: Handle exception?
                func(actualValue);
            });

        if (autoInvoke) {
            func(this->getValue());
        }

        this->managedConnections.emplace_back(std::move(connection));
    }

    template <typename ConnectionManager>
    void
    connect(std::function<void(const Type &)> func,
            ConnectionManager &userDefinedManagedConnections,
            bool autoInvoke = true)
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            return;
        }

        auto connection = lockedSetting->updated.connect(
            [=](const rapidjson::Value &value, const SignalArgs &) {
                func(Deserialize<Type>::get(value));  //
            });

        if (autoInvoke) {
            func(this->getValue());
        }

        userDefinedManagedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
    }

    // Connect: no args
    void
    connect(std::function<void()> func, bool autoInvoke = true)
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            return;
        }

        auto connection = lockedSetting->updated.connect(
            [=](const rapidjson::Value &, const SignalArgs &) {
                func();  //
            });

        if (autoInvoke) {
            func();
        }

        this->managedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
    }

    template <typename ConnectionManager>
    void
    connect(std::function<void()> func,
            ConnectionManager &userDefinedManagedConnections,
            bool autoInvoke = true)
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            return;
        }

        auto connection = lockedSetting->updated.connect(
            [=](const rapidjson::Value &, const SignalArgs &) {
                func();  //
            });

        if (autoInvoke) {
            func();
        }

        userDefinedManagedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
    }

private:
    std::vector<std::unique_ptr<Signals::ScopedConnection>> managedConnections;
};

}  // namespace Settings
}  // namespace pajlada
