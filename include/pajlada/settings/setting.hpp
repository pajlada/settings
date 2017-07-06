#pragma once

#include "pajlada/settings/borrowedsetting.hpp"
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
    Setting(const std::string &path,
            SettingOption options = SettingOption::Default);

    // Path, Default Value
    Setting(const std::string &path, const Type &defaultValue,
            SettingOption options = SettingOption::Default);

    virtual ~Setting() = default;

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

    BorrowedSetting<Type>
    borrow()
    {
        return BorrowedSetting<Type>(std::shared_ptr<Container>(this->data));
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

    std::shared_ptr<Container> data;

public:
    Signals::Signal<const Type &> &
    getValueChangedSignal()
    {
        return this->data->valueChanged;
    };

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

    friend class ISettingData;
};

// Path
template <typename Type, typename Container>
Setting<Type, Container>::Setting(const std::string &path,
                                  SettingOption options)
    : data(SettingManager::createSetting<Type, Container>(path, options))
{
}

// Path, Default Value
template <typename Type, typename Container>
Setting<Type, Container>::Setting(const std::string &path,
                                  const Type &defaultValue,
                                  SettingOption options)
    : data(SettingManager::createSetting<Type, Container>(path, defaultValue,
                                                          options))
{
}

}  // namespace Settings
}  // namespace pajlada
