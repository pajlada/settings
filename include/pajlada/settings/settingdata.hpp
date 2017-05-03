#pragma once

#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <pajlada/signals/signal.hpp>

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace pajlada {
namespace Settings {

template <typename Type>
class Setting;

namespace detail {

class ISettingData
{
public:
    ISettingData();

    virtual ~ISettingData() = default;

    inline uint64_t
    getConnectionID() const
    {
        return this->connectionID;
    }

    const std::string &getPath() const;

    void setPath(const std::string &_path);
    void setKey(const std::string &_key, const Setting<Object> &parent);
    void setIndex(unsigned _index, const Setting<Array> &parent);

private:
    // Setting path (i.e. /a/b/c/3/d/e)
    std::string path;

    uint64_t connectionID = 0;

protected:
    // If the setting has been filled with any value other than the default
    // one
    bool filled = false;
};

template <typename Type>
class SettingData : public ISettingData,
                    public std::enable_shared_from_this<SettingData<Type>>
{
    SettingData(const Type &defaultValue)
        : ISettingData()
        , value(defaultValue)
    {
    }

public:
    void
    setValue(const Type &newValue)
    {
        this->value = newValue;

        this->filled = true;

        this->valueChanged.invoke(newValue);
    }

    Type
    getValue() const
    {
        return this->value;
    }

    signals::Signal<const Type &> valueChanged;

private:
    Type value;

    friend class Setting<Type>;
};

}  // namespace detail
}  // namespace setting
}  // namespace pajlada
