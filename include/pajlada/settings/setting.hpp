#pragma once

#include <pajlada/settings/common.hpp>
#include <pajlada/settings/equal.hpp>
#include <pajlada/settings/settingdata.hpp>
#include <pajlada/settings/settingmanager.hpp>

#include <pajlada/signals.hpp>
#include <rapidjson/document.h>

#include <iostream>
#include <mutex>
#include <type_traits>

namespace pajlada {
namespace Settings {

namespace {

SignalArgs
onConnectArgs()
{
    static SignalArgs a = []() {
        SignalArgs v;
        v.source = SignalArgs::Source::OnConnect;

        return v;
    }();

    return a;
}

}  // namespace

// A default value passed to a setting is only local to this specific instance of the setting
// it is never shared between other settings at the same path
template <typename Type>
class Setting
{
    const std::string path;

public:
    explicit Setting(const std::string &_path,
                     SettingOption _options = SettingOption::Default,
                     std::shared_ptr<SettingManager> instance = nullptr)
        : path(_path)
        , data(SettingManager::getSetting(_path, instance))
        , options(_options)
    {
    }

    explicit Setting(const char *_path,
                     SettingOption _options = SettingOption::Default,
                     std::shared_ptr<SettingManager> instance = nullptr)
        : path(_path)
        , data(SettingManager::getSetting(_path, instance))
        , options(_options)
    {
    }

    explicit Setting(const std::string &_path, Type _defaultValue,
                     SettingOption _options = SettingOption::Default,
                     std::shared_ptr<SettingManager> instance = nullptr)
        : path(_path)
        , data(SettingManager::getSetting(_path, instance))
        , options(_options)
        , defaultValue(std::move(_defaultValue))
    {
    }

    explicit Setting(const char *_path, Type _defaultValue,
                     SettingOption _options = SettingOption::Default,
                     std::shared_ptr<SettingManager> instance = nullptr)
        : path(_path)
        , data(SettingManager::getSetting(_path, instance))
        , options(_options)
        , defaultValue(std::move(_defaultValue))
    {
    }

    explicit Setting(const std::string &_path,
                     std::shared_ptr<SettingManager> instance)
        : path(_path)
        , data(SettingManager::getSetting(_path, instance))
    {
    }

    explicit Setting(const char *_path,
                     std::shared_ptr<SettingManager> instance)
        : path(_path)
        , data(SettingManager::getSetting(_path, instance))
    {
    }

    explicit Setting(const std::string &_path, Type _defaultValue,
                     std::shared_ptr<SettingManager> instance)
        : path(_path)
        , data(SettingManager::getSetting(_path, instance))
        , defaultValue(std::move(_defaultValue))
    {
    }

    explicit Setting(const char *_path, Type _defaultValue,
                     std::shared_ptr<SettingManager> instance)
        : path(_path)
        , data(SettingManager::getSetting(_path, instance))
        , defaultValue(std::move(_defaultValue))
    {
    }

    // Copy constructor
    Setting(const Setting &other)
        : path(other.path)
        , data(other.data)
        , options(other.options)
        , defaultValue(other.defaultValue)
        , value(other.value)
        , updateIteration(other.updateIteration)
    {
        // managedConnections is not copied on purpose
        // valueMutex is not copied on purpose
    }

    inline bool
    optionEnabled(SettingOption option) const
    {
        return (this->options & option) == option;
    }

    ~Setting() = default;

    bool
    isValid() const
    {
        return !this->data.expired();
    }

    const std::string &
    getPath() const
    {
        return this->path;
    }

    Type
    getValueCopy() const
    {
        return this->getValue();
    }

    const Type &
    getValue() const
    {
        std::unique_lock<std::mutex> lock(this->valueMutex);

        auto lockedSetting = this->data.lock();

        if (!lockedSetting) {
            if (this->value) {
                return *this->value;
            }

            return this->defaultValue;
        }

        if (this->updateIteration == lockedSetting->getUpdateIteration()) {
            // Value hasn't been updated
            if (this->value) {
                return *this->value;
            }

            return this->defaultValue;
        }

        auto p = lockedSetting->template unmarshal<Type>();
        if (p.value) {
            this->value = p.value;
            this->updateIteration = p.updateIteration;
        }

        if (this->value) {
            return *this->value;
        }

        return this->defaultValue;
    }

    template <typename T = Type,
              typename = std::enable_if_t<is_stl_container<T>::value>>
    const Type &
    getArray() const
    {
        auto lockedSetting = this->data.lock();

        if (!lockedSetting) {
            if (this->defaultValue) {
                return *this->defaultValue;
            }

            return this->value;
        }

        auto p = lockedSetting->template unmarshal<Type>();
        if (p.second) {
            return p.first;
        }

        if (this->defaultValue) {
            return *this->defaultValue;
        }

        return p.first;
    }

    // Implement vector helper stuff
    template <typename T = Type,
              typename = std::enable_if_t<is_stl_container<T>::value>>
    void
    push_back(typename T::value_type newItem, SignalArgs &&args = SignalArgs())
    {
        // TODO: refresh this->value first?
        this->valueMutex.lock();
        if (!this->value) {
            this->value = Type{};
        }

        (*this->value).push_back(std::move(newItem));
        auto copy = *this->value;
        this->valueMutex.unlock();
        this->updateValue(copy, std::move(args));
    }

    template <typename T = Type,
              typename = std::enable_if_t<is_stl_container<T>::value>>
    void
    removeByValue(const typename T::value_type &key,
                  SignalArgs &&args = SignalArgs())
    {
        // TODO: refresh this->value first?

        T copy;

        {
            std::unique_lock<std::mutex> lock(this->valueMutex);
            if (!this->value) {
                return;
            }

            copy = (*this->value);
        }

        auto valueSize = copy.size();

        copy.erase(std::remove(copy.begin(), copy.end(), key), copy.end());

        if (copy.size() == valueSize) {
            // nothing was removed
            return;
        }

        {
            this->valueMutex.lock();
            this->value = copy;
            this->valueMutex.unlock();
            this->updateValue(copy, std::move(args));
        }
    }

private:
    bool
    updateValue(const Type &newValue, SignalArgs &&args)
    {
        if (this->optionEnabled(SettingOption::DoNotWriteToJSON)) {
            args.writeToFile = false;
        }

        auto lockedSetting = this->data.lock();

        if (lockedSetting) {
            if (args.source == SignalArgs::Source::Unset) {
                args.source = SignalArgs::Source::Setter;
            }
            return lockedSetting->marshal(newValue, std::move(args));
        }

        return false;
    }

public:
    bool
    setValue(const Type &newValue, SignalArgs &&args = SignalArgs())
    {
        {
            std::unique_lock<std::mutex> lock(this->valueMutex);
            this->value = newValue;
        }

        if (this->optionEnabled(SettingOption::DoNotWriteToJSON)) {
            args.writeToFile = false;
        }

        auto lockedSetting = this->data.lock();

        if (lockedSetting) {
            if (args.source == SignalArgs::Source::Unset) {
                args.source = SignalArgs::Source::Setter;
            }
            return lockedSetting->marshal(newValue, std::move(args));
        }

        return false;
    }

    Setting &
    operator=(const Type &newValue)
    {
        this->setValue(newValue);

        return *this;
    }

    template <typename T2>
    Setting &
    operator=(const T2 &newValue)
    {
        this->setValue(Type(newValue));

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

    operator Type() const
    {
        assert(this->isValid());

        return this->getValue();
    }

    void
    resetToDefaultValue(SignalArgs &&args = SignalArgs())
    {
        this->setValue(this->defaultValue, std::move(args));
    }

    void
    setDefaultValue(const Type &newDefaultValue)
    {
        this->defaultValue = newDefaultValue;
    }

    Type
    getDefaultValue() const
    {
        return this->defaultValue;
    }

    // Returns true if the current value is the same as the default value
    // boost::any cannot be properly compared
    bool
    isDefaultValue() const
    {
        return IsEqual<Type>::get(this->getValue(), this->getDefaultValue());
    }

    // Remove will invalidate this setting and all other settings that point at the same path
    // If the setting is an object or array, any child settings will also be invalidated
    // the remove function handles the exception handling in case this setting is already invalid
    bool
    remove()
    {
        return SettingManager::removeSetting(this->getPath());
    }

private:
    std::weak_ptr<SettingData> data;
    SettingOption options = SettingOption::Default;
    Type defaultValue{};

    // These are mutable because they can be modified from the "getValue" function
    mutable std::mutex valueMutex;
    mutable OptionalType<Type> value;
    mutable int updateIteration = -1;

public:
    std::weak_ptr<SettingData>
    getData()
    {
        return this->data;
    }

    // ConnectJSON: Connect with rapidjson::Value and SignalArgs as arguments
    // No deserialization is made by the setting
    void
    connectJSON(
        std::function<void(const rapidjson::Value &, const SignalArgs &)> func,
        bool autoInvoke = true)
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            return;
        }

        auto connection = lockedSetting->updated.connect(func);

        if (autoInvoke) {
            auto ptr = lockedSetting->unmarshalJSON();
            rapidjson::Document d;
            if (ptr != nullptr) {
                d.CopyFrom(*ptr, d.GetAllocator());
            }
            connection.invoke(std::move(d), onConnectArgs());
        }

        this->managedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
    }

    template <typename ConnectionManager>
    void
    connectJSON(
        std::function<void(const rapidjson::Value &, const SignalArgs &)> func,
        ConnectionManager &userDefinedManagedConnections,
        bool autoInvoke = true)
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            return;
        }

        auto connection = lockedSetting->updated.connect(func);

        if (autoInvoke) {
            auto ptr = lockedSetting->unmarshalJSON();
            rapidjson::Document d;
            if (ptr != nullptr) {
                d.CopyFrom(*ptr, d.GetAllocator());
            }
            connection.invoke(std::move(d), onConnectArgs());
        }

        userDefinedManagedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
    }

    // Connect: Value and SignalArgs
    void
    connect(std::function<void(const Type &, const SignalArgs &)> func,
            bool autoInvoke = true)
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            return;
        }

        auto connection = lockedSetting->updated.connect(
            [=](const rapidjson::Value &value, const SignalArgs &args) {
                func(Deserialize<Type>::get(value), args);  //
            });

        if (autoInvoke) {
            func(this->getValue(), onConnectArgs());
        }

        this->managedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
    }

    template <typename ConnectionManager>
    void
    connect(std::function<void(const Type &, const SignalArgs &)> func,
            ConnectionManager &userDefinedManagedConnections,
            bool autoInvoke = true)
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            return;
        }

        auto connection = lockedSetting->updated.connect(
            [=](const rapidjson::Value &value, const SignalArgs &args) {
                func(Deserialize<Type>::get(value), args);  //
            });

        if (autoInvoke) {
            func(this->getValue(), onConnectArgs());
        }

        userDefinedManagedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
    }

    // Connect: Value
    void
    connect(std::function<void(const Type &)> func, bool autoInvoke = true)
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

        this->managedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
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

    // ConnectSimple: Signal args only
    void
    connectSimple(std::function<void(const SignalArgs &)> func,
                  bool autoInvoke = true)
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            return;
        }

        auto connection = lockedSetting->updated.connect(
            [=](const rapidjson::Value &, const SignalArgs &args) {
                func(args);  //
            });

        if (autoInvoke) {
            func(onConnectArgs());
        }

        this->managedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
    }

    template <typename ConnectionManager>
    void
    connectSimple(std::function<void(const SignalArgs &)> func,
                  ConnectionManager &userDefinedManagedConnections,
                  bool autoInvoke = true)
    {
        auto lockedSetting = this->data.lock();
        if (!lockedSetting) {
            return;
        }

        auto connection = lockedSetting->updated.connect(
            [=](const rapidjson::Value &, const SignalArgs &args) {
                func(args);  //
            });

        if (autoInvoke) {
            func(onConnectArgs());
        }

        userDefinedManagedConnections.emplace_back(
            std::make_unique<Signals::ScopedConnection>(std::move(connection)));
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
    std::vector<std::unique_ptr<Signals::ScopedConnection>> managedConnections;
};

}  // namespace Settings
}  // namespace pajlada
