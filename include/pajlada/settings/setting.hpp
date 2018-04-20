#pragma once

#include "pajlada/settings/equal.hpp"
#include "pajlada/settings/exception.hpp"
#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"

#include <rapidjson/document.h>
#include <pajlada/signals/signal.hpp>

#include <memory>
#include <string>

namespace pajlada {
namespace Settings {

template <typename Type>
class Setting
{
    using Container = SettingData<Type>;

public:
    // Path, Setting Options
    Setting(const std::string &path,
            SettingOption options = SettingOption::Default);

    // Path, Default Value, Setting Options
    Setting(const std::string &path, const Type &defaultValue,
            SettingOption options = SettingOption::Default);

    // Path, Default Value, Current Value, Setting Options
    Setting(const std::string &path, const Type &defaultValue,
            const Type &currentValue,
            SettingOption options = SettingOption::Default);

    ~Setting() = default;

    bool
    isValid() const
    {
        return !this->data.expired();
    }

    std::string
    getPath() const
    {
        auto lockedSetting = this->getLockedData();

        return lockedSetting->getPath();
    }

    const Type
    getValue() const
    {
        auto lockedSetting = this->getLockedData();

        return lockedSetting->getValue();
    }

    bool
    hasBeenSet() const
    {
        auto lockedSetting = this->getLockedData();

        return lockedSetting->hasBeenSet();
    }

    void
    setValue(const Type &newValue, SignalArgs &&args = SignalArgs())
    {
        auto lockedSetting = this->getLockedData();

        lockedSetting->setValue(newValue, std::move(args));
    }

    Setting &
    operator=(const Type &newValue)
    {
        auto lockedSetting = this->getLockedData();

        this->setValue(newValue);

        return *this;
    }

    template <typename T2>
    Setting &
    operator=(const T2 &newValue)
    {
        auto lockedSetting = this->getLockedData();

        this->setValue(newValue);

        return *this;
    }

    Setting &
    operator=(Type &&newValue)
    {
        this->setValue(std::move(newValue));

        return *this;
    }

    bool
    operator==(const Type &rhs) const
    {
        assert(this->isValid());

        return this->getValue() == rhs;
    }

    bool
    operator!=(const Type &rhs) const
    {
        assert(this->isValid());

        return this->getValue() != rhs;
    }

    operator const Type() const
    {
        assert(this->isValid());

        return this->getValue();
    }

    void
    resetToDefaultValue(SignalArgs &&args = SignalArgs())
    {
        auto lockedSetting = this->getLockedData();

        lockedSetting->resetToDefaultValue(std::move(args));
    }

    void
    setDefaultValue(const Type &newDefaultValue)
    {
        auto lockedSetting = this->getLockedData();

        lockedSetting->setDefaultValue(newDefaultValue);
    }

    Type
    getDefaultValue() const
    {
        auto lockedSetting = this->getLockedData();

        return lockedSetting->getDefaultValue();
    }

    // Returns true if the current value is the same as the default value
    // boost::any cannot be properly compared
    bool
    isDefaultValue() const
    {
        auto lockedSetting = this->getLockedData();

        return IsEqual<Type>::get(this->getValue(), this->getDefaultValue());
    }

    // Remove will invalidate this setting and all other settings that point at the same path
    // If the setting is an object or array, any child settings will also be invalidated
    // the remove function handles the exception handling in case this setting is already invalid
    bool
    remove()
    {
        try {
            SettingManager::removeSetting(this->getPath());
        } catch (const Exception &) {
            return false;
        }

        return true;
    }

protected:
    std::weak_ptr<Container> data;

private:
    // getLockedData is an internal helper function
    // It will either return a valid shared_ptr to the underlying SettingData, or throw an exception
    std::shared_ptr<Container>
    getLockedData() const
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            throw Exception(Exception::ExpiredSetting);
        }

        return lockedSetting;
    }

public:
    Signals::Signal<const Type &, const SignalArgs &> &
    getValueChangedSignal()
    {
        auto lockedSetting = this->getLockedData();

        return lockedSetting->valueChanged;
    }

    Signals::Signal<const SignalArgs &> &
    getSimpleSignal()
    {
        auto lockedSetting = this->getLockedData();

        return lockedSetting->valueChanged;
    }

    std::weak_ptr<Container>
    getData()
    {
        return this->data;
    }

    void
    connect(typename Container::valueChangedCallbackType func,
            bool autoInvoke = true)
    {
        auto lockedSetting = this->getLockedData();

        auto connection = lockedSetting->valueChanged.connect(func);

        if (autoInvoke) {
            SignalArgs invocationArgs;
            invocationArgs.source = SignalArgs::Source::OnConnect;

            connection.invoke(lockedSetting->getValue(), invocationArgs);
        }

        this->managedConnections.emplace_back(std::move(connection));
    }

    void
    connectSimple(std::function<void(const SignalArgs &)> func,
                  bool autoInvoke = true)
    {
        auto lockedSetting = this->getLockedData();

        auto connection = lockedSetting->simpleValueChanged.connect(func);

        if (autoInvoke) {
            SignalArgs invocationArgs;
            invocationArgs.source = SignalArgs::Source::OnConnect;

            connection.invoke(invocationArgs);
        }

        this->managedConnections.emplace_back(std::move(connection));
    }

    void
    connect(
        typename Container::valueChangedCallbackType func,
        std::vector<Signals::ScopedConnection> &userDefinedManagedConnections,
        bool autoInvoke = true)
    {
        auto lockedSetting = this->getLockedData();

        auto connection = lockedSetting->valueChanged.connect(func);

        if (autoInvoke) {
            SignalArgs invocationArgs;
            invocationArgs.source = SignalArgs::Source::OnConnect;

            connection.invoke(lockedSetting->getValue(), invocationArgs);
        }

        userDefinedManagedConnections.emplace_back(std::move(connection));
    }

    void
    connectSimple(
        std::function<void(const SignalArgs &)> func,
        std::vector<Signals::ScopedConnection> &userDefinedManagedConnections,
        bool autoInvoke = true)
    {
        auto lockedSetting = this->getLockedData();

        auto connection = lockedSetting->simpleValueChanged.connect(func);

        if (autoInvoke) {
            SignalArgs invocationArgs;
            invocationArgs.source = SignalArgs::Source::OnConnect;

            connection.invoke(invocationArgs);
        }

        userDefinedManagedConnections.emplace_back(std::move(connection));
    }

    // Static helper methods for one-offs (get or set setting)
    static const Type
    get(const std::string &path, SettingOption options = SettingOption::Default)
    {
        Setting<Type> setting(path, options);

        return setting.getValue();
    }

    static void
    set(const std::string &path, const Type &newValue,
        SettingOption options = SettingOption::Default)
    {
        Setting<Type> setting(path, options);

        setting.setValue(newValue);
    }

private:
    std::vector<Signals::ScopedConnection> managedConnections;

    friend class ISettingData;
};

// Path, Setting Options
template <typename Type>
Setting<Type>::Setting(const std::string &path, SettingOption options)
    : data(SettingManager::createSetting<Type, Container>(path, options))
{
}

// Path, Default Value, Setting Options
template <typename Type>
Setting<Type>::Setting(const std::string &path, const Type &defaultValue,
                       SettingOption options)
    : data(SettingManager::createSetting<Type, Container>(path, defaultValue,
                                                          options))
{
}

// Path, Default Value, Current Value, Setting Options
template <typename Type>
Setting<Type>::Setting(const std::string &path, const Type &defaultValue,
                       const Type &currentValue, SettingOption options)
    : data(SettingManager::createSetting<Type, Container>(
          path, defaultValue, currentValue, options))
{
}

}  // namespace Settings
}  // namespace pajlada
