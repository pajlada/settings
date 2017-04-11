#pragma once

#include "pajlada/settings/jsonwrapper.hpp"
#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <pajlada/signals/signal.hpp>

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

class ISettingData
{
public:
    ISettingData(const std::string &_key, Setting<Object> *_parent);
    ISettingData(unsigned _index, Setting<Array> *_parent);
    ISettingData();

    virtual ~ISettingData() = default;

    inline uint64_t
    getConnectionID() const
    {
        return this->connectionID;
    }

    Setting<Object> *
    getSettingObjectParent() const
    {
        return this->settingObjectParent;
    }

    Setting<Array> *
    getSettingArrayParent() const
    {
        return this->settingArrayParent;
    }

    inline bool
    isFilled() const
    {
        return this->filled;
    }

    const std::string &getPath() const;
    const std::string &getKey() const;
    unsigned getIndex() const;

    void setPath(const std::string &_path);
    void setKey(const std::string &_key, const Setting<Object> &parent);
    void setIndex(unsigned _index, const Setting<Array> &parent);

private:
    // Setting path (i.e. /a/b/c/3/d/e)
    std::string path;

    // Setting key (i.e. "numThreads")
    std::string key;

    // Setting index (i.e. 2)
    unsigned index;

protected:
    // If the setting has been filled with any value other than the default
    // one
    bool filled = false;

    Setting<Object> *settingObjectParent;
    Setting<Array> *settingArrayParent;

    uint64_t connectionID = 0;
    static std::atomic<uint64_t> latestConnectionID;
};

template <typename Type>
class SettingData : public ISettingData,
                    public std::enable_shared_from_this<SettingData<Type>>
{
    // Default Value
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

}  // namespace setting
}  // namespace pajlada
