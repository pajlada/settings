#pragma once

#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <pajlada/signals/signal.hpp>

#include <memory>

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
public:
    // Path, Default Value
    Setting(const std::string &path, const Type &defaultValue = Type());

    // Key, Object Parent, Default Value
    Setting(const std::string &key, const Setting<Object> &parent,
            const Type &defaultValue = Type());

    // Index, Array Parent, Default Value
    Setting(unsigned index, const Setting<Array> &parent,
            const Type &defaultValue = Type());

    ~Setting();

    Setting &
    setName(const char *newName)
    {
        this->name = newName;

        return *this;
    }

    const Type
    getValue() const
    {
        assert(this->data != nullptr);

        return this->data->getValue();
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
        this->data->setValue(std::move(newValue));

        return *this;
    }

    operator const Type() const
    {
        return this->getValue();
    }

private:
    std::shared_ptr<detail::SettingData<Type>>
    getData() const
    {
        return this->data;
    }

    void
    registerSetting()
    {
        SettingManager::registerSetting(this->data);
    }

    void
    unregisterSetting()
    {
        SettingManager::unregisterSetting(this->data);
    }

    std::shared_ptr<detail::SettingData<Type>> data;

public:
    Signals::Signal<const Type &> &valueChanged;

private:
    std::string name;

    friend class detail::ISettingData;
};

// Path, Default Value
template <typename Type>
Setting<Type>::Setting(const std::string &path, const Type &defaultValue)
    : data(new detail::SettingData<Type>(defaultValue))
    , valueChanged(data->valueChanged)
{
    this->data->setPath(path);

    this->registerSetting();
}

// Key, Object Parent, Default Value
template <typename Type>
Setting<Type>::Setting(const std::string &key, const Setting<Object> &parent,
                       const Type &defaultValue)
    : data(new detail::SettingData<Type>(defaultValue))
    , valueChanged(data->valueChanged)
{
    this->data->setKey(key, parent);

    this->registerSetting();
}

// Index, Array Parent, Default Value
template <typename Type>
Setting<Type>::Setting(unsigned index, const Setting<Array> &parent,
                       const Type &defaultValue)
    : data(new detail::SettingData<Type>(defaultValue))
    , valueChanged(data->valueChanged)
{
    this->data->setIndex(index, parent);

    this->registerSetting();
}

template <typename Type>
Setting<Type>::~Setting()
{
    this->unregisterSetting();
}

}  // namespace Settings
}  // namespace pajlada
