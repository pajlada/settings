#pragma once

#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace pajlada {
namespace settings {

template <typename Type>
class Setting;

class ISetting
{
public:
    virtual ~ISetting() = default;

private:
    // Setting description (i.e. Number of threads to run the application
    // in)
    std::string description;
};

template <typename Type>
class Setting : public ISetting
{
public:
    // Key, Default Value, Object Parent
    Setting(const std::string &key, const Type &defaultValue = Type(),
            Setting<Object> *parent = nullptr);

    // Index, Default Value, Array Parent
    Setting(unsigned index, const Type &defaultValue = Type(),
            Setting<Array> *parent = nullptr);

    // Key, Object Parent
    Setting(const std::string &key, Setting<Object> *parent);

    // Index, Array Parent
    Setting(unsigned index, Setting<Array> *parent);

    ~Setting();

    Setting &
    setName(const char *newName)
    {
        this->name = newName;

        return *this;
    }

    std::shared_ptr<SettingData<Type>>
    getData() const
    {
        return this->data;
    }

    const std::string &
    getKey() const
    {
        return this->data->getKey();
    }

    const std::string &
    getName() const
    {
        return this->name;
    }

    const Type
    getValue() const
    {
        assert(this->data != nullptr);

        return this->data->getValue();
    }

    const Type
    get() const
    {
        return this->getValue();
    }

    void
    setValue(const Type &newValue)
    {
        this->data->setValue(newValue);
    }

    Setting &
    operator=(const Type &newValue)
    {
        this->data->setValue(newValue);

        return *this;
    }

    Setting &
    operator=(Type &&newValue) noexcept
    {
        this->data->setValue(newValue);

        return *this;
    }

    operator const Type() const
    {
        return this->getValue();
    }

private:
    std::shared_ptr<SettingData<Type>> data;

    std::string name;
};

// Key, Default Value, Object Parent
template <typename Type>
Setting<Type>::Setting(const std::string &key, const Type &defaultValue,
                       Setting<Object> *parent)
    : data(new SettingData<Type>(key, defaultValue, parent))
{
    SettingManager::registerSetting(this->data);
}

// Index, Default Value, Array Parent
template <typename Type>
Setting<Type>::Setting(unsigned index, const Type &defaultValue,
                       Setting<Array> *parent)
    : data(new SettingData<Type>(index, defaultValue, parent))
{
    SettingManager::registerSetting(this->data);
}

// Key, Object Parent
template <typename Type>
Setting<Type>::Setting(const std::string &key, Setting<Object> *parent)
    : data(new SettingData<Type>(key, parent))
{
    SettingManager::registerSetting(this->data);
}

// Index, Array Parent
template <typename Type>
Setting<Type>::Setting(unsigned index, Setting<Array> *parent)
    : data(new SettingData<Type>(index, parent))
{
    SettingManager::registerSetting(this->data);
}

template <typename Type>
Setting<Type>::~Setting()
{
    SettingManager::unregisterSetting(this->data);
}

}  // namespace setting
}  // namespace pajlada
