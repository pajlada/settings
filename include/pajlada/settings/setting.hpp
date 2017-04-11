#pragma once

#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

#include <memory>

namespace pajlada {
namespace settings {

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

    const std::string &
    getPath() const
    {
        return this->path;
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

    std::string path;

    std::shared_ptr<SettingData<Type>> data;

    std::string name;
};

// Path, Default Value
template <typename Type>
Setting<Type>::Setting(const std::string &path, const Type &defaultValue)
    : data(new SettingData<Type>(defaultValue))
{
    this->data->setPath(path);

    this->registerSetting();
}

// Key, Object Parent, Default Value
template <typename Type>
Setting<Type>::Setting(const std::string &key, const Setting<Object> &parent,
                       const Type &defaultValue)
    : data(new SettingData<Type>(defaultValue))
{
    this->data->setKey(key, parent);

    this->registerSetting();
}

// Index, Array Parent, Default Value
template <typename Type>
Setting<Type>::Setting(unsigned index, const Setting<Array> &parent,
                       const Type &defaultValue = Type())
    : data(new SettingData<Type>(defaultValue))
{
    this->data->setIndex(index, parent);

    this->registerSetting();
}

template <typename Type>
Setting<Type>::~Setting()
{
    this->unregisterSetting();
}

}  // namespace setting
}  // namespace pajlada
