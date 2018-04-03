#pragma once

#include "pajlada/settings/equal.hpp"
#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <pajlada/signals/signal.hpp>

#include <memory>
#include <string>

namespace pajlada {
namespace Settings {

namespace detail {

class ISetting
{
public:
    virtual ~ISetting() = default;

private:
    // Setting description (i.e. Number of threads to run the application
    // in)
    std::string description;
};

}  // namespace detail

template <typename Type>
class Setting : public detail::ISetting
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

    ~Setting() override = default;

    const std::string &
    getPath() const
    {
        assert(this->data != nullptr);

        return this->data->getPath();
    }

    Setting &
    setName(const char *newName)
    {
        assert(this->data != nullptr);

        this->name = newName;

        return *this;
    }

    const Type
    getValue() const
    {
        assert(this->data != nullptr);

        return this->data->getValue();
    }

    bool
    hasBeenSet() const
    {
        assert(this->data != nullptr);

        return this->data->hasBeenSet();
    }

    void
    setValue(const Type &newValue, SignalArgs &&args = SignalArgs())
    {
        assert(this->data != nullptr);

        this->data->setValue(newValue, std::move(args));
    }

    Setting &
    operator=(const Type &newValue)
    {
        assert(this->data != nullptr);

        this->setValue(newValue);

        return *this;
    }

    template <typename T2>
    Setting &
    operator=(const T2 &newValue)
    {
        assert(this->data != nullptr);

        this->setValue(newValue);

        return *this;
    }

    Setting &
    operator=(Type &&newValue) noexcept
    {
        assert(this->data != nullptr);

        this->setValue(std::move(newValue));

        return *this;
    }

    bool
    operator==(const Type &rhs) const
    {
        assert(this->data != nullptr);

        return this->getValue() == rhs;
    }

    bool
    operator!=(const Type &rhs) const
    {
        assert(this->data != nullptr);

        return this->getValue() != rhs;
    }

    operator const Type() const
    {
        assert(this->data != nullptr);

        return this->getValue();
    }

    void
    resetToDefaultValue(SignalArgs &&args = SignalArgs())
    {
        assert(this->data != nullptr);

        this->data->resetToDefaultValue(std::move(args));
    }

    void
    setDefaultValue(const Type &newDefaultValue)
    {
        assert(this->data != nullptr);

        this->data->setDefaultValue(newDefaultValue);
    }

    Type
    getDefaultValue() const
    {
        assert(this->data != nullptr);

        return this->data->getDefaultValue();
    }

    // Returns true if the current value is the same as the default value
    // boost::any cannot be properly compared
    bool
    isDefaultValue() const
    {
        assert(this->data != nullptr);

        return IsEqual<Type>::get(this->getValue(), this->getDefaultValue());
    }

    // Remove this setting
    bool
    remove()
    {
        auto uc = this->data.use_count();

        if (uc != 2) {
            return false;
        }

        this->managedConnections.clear();

        SettingManager::removeSetting(this->getPath());

        this->data.reset();

        return true;
    }

protected:
    std::shared_ptr<Container> data;

public:
    Signals::Signal<const Type &, const SignalArgs &> &
    getValueChangedSignal()
    {
        assert(this->data != nullptr);

        return this->data->valueChanged;
    }

    Signals::Signal<const SignalArgs &> &
    getSimpleSignal()
    {
        assert(this->data != nullptr);

        return this->data->simpleValueChanged;
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
        assert(this->data != nullptr);

        auto connection = this->data->valueChanged.connect(func);

        if (autoInvoke) {
            SignalArgs invocationArgs;
            invocationArgs.source = SignalArgs::Source::OnConnect;

            connection.invoke(this->data->getValue(), invocationArgs);
        }

        this->managedConnections.emplace_back(std::move(connection));
    }

    void
    connectSimple(std::function<void(const SignalArgs &)> func,
                  bool autoInvoke = true)
    {
        assert(this->data != nullptr);

        auto connection = this->data->simpleValueChanged.connect(func);

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
        assert(this->data != nullptr);

        auto connection = this->data->valueChanged.connect(func);

        if (autoInvoke) {
            SignalArgs invocationArgs;
            invocationArgs.source = SignalArgs::Source::OnConnect;

            connection.invoke(this->data->getValue(), invocationArgs);
        }

        userDefinedManagedConnections.emplace_back(std::move(connection));
    }

    void
    connectSimple(
        std::function<void(const SignalArgs &)> func,
        std::vector<Signals::ScopedConnection> &userDefinedManagedConnections,
        bool autoInvoke = true)
    {
        assert(this->data != nullptr);

        auto connection = this->data->simpleValueChanged.connect(func);

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
    std::string name;

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
