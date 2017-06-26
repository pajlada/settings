#pragma once

#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/types.hpp"

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

template <typename Type, typename Container = SettingData<Type>>
class Setting : public detail::ISetting
{
public:
    // Path
    Setting(const std::string &path);

    // Path, Default Value
    Setting(const std::string &path, const Type &defaultValue);

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

    const Type &
    getValueRef() const
    {
        assert(this->data != nullptr);

        return this->data->getValueRef();
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

    template <typename T2>
    Setting &
    operator=(const T2 &newValue)
    {
        this->data->setValue(newValue);

        return *this;
    }

    Container *operator->()
    {
        this->data->dirty = true;

        return this->data.get();
    }

    Setting &
    operator=(Type &&newValue) noexcept
    {
        this->data->setValue(std::move(newValue));

        return *this;
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

    operator const Type() const
    {
        return this->getValue();
    }

private:
    std::shared_ptr<Container>
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

    std::shared_ptr<Container> data;

public:
    Signals::Signal<const Type &> &
    getValueChangedSignal()
    {
        return this->data->valueChanged;
    };

private:
    std::string name;

    friend class ISettingData;
};

// Path
template <typename Type, typename Container>
Setting<Type, Container>::Setting(const std::string &path)
    : data(new Container())
{
    this->data->setPath(path);

    this->registerSetting();
}

// Path, Default Value
template <typename Type, typename Container>
Setting<Type, Container>::Setting(const std::string &path,
                                  const Type &defaultValue)
    : data(new Container(defaultValue))
{
    this->data->setPath(path);

    this->registerSetting();
}

template <typename Type, typename Container>
Setting<Type, Container>::~Setting()
{
    this->unregisterSetting();
}

}  // namespace Settings
}  // namespace pajlada
