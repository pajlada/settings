#pragma once

#include "pajlada/settings/jsonwrapper.hpp"
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

class ISettingData
{
public:
    ISettingData(const std::string &_key, Setting<Object> *_parent);
    ISettingData(unsigned _index, Setting<Array> *_parent);

    inline uint64_t
    getConnectionID() const
    {
        return this->connectionID;
    }

    virtual ~ISettingData() = default;

    rapidjson::Value *
    getJSONParent() const
    {
        return this->jsonParent;
    }

    rapidjson::Value *
    getJSONValue() const
    {
        return this->jsonValue;
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

    void
    setJSONParent(rapidjson::Value *newParent)
    {
        this->jsonParent = newParent;
    }

    const std::string &
    getKey() const
    {
        return this->key;
    }

    unsigned
    getIndex() const
    {
        return this->index;
    }

    inline bool
    isFilled() const
    {
        return this->filled;
    }

    void
    setJSONValue(rapidjson::Value *newValue)
    {
        this->jsonValue = newValue;
    }

private:
    // Setting key (i.e. "numThreads")
    const std::string key;

    // Setting index (i.e. 2)
    const unsigned index;

protected:
    // If the setting has been filled with any value other than the default
    // one
    bool filled = false;

    rapidjson::Value *jsonParent = nullptr;
    rapidjson::Value *jsonValue = nullptr;
    Setting<Object> *settingObjectParent;
    Setting<Array> *settingArrayParent;

    uint64_t connectionID = 0;
    static std::atomic<uint64_t> latestConnectionID;
};

template <typename Type>
class SettingData : public ISettingData
{
public:
    // Key, Default Value, Object Parent
    SettingData(const std::string &_key, const Type &defaultValue,
                Setting<Object> *_parent)
        : ISettingData(_key, _parent)
        , data(defaultValue)
    {
    }

    // Index, Default Value, Array Parent
    SettingData(unsigned _index, const Type &defaultValue,
                Setting<Array> *_parent)
        : ISettingData(_index, _parent)
        , data(defaultValue)
    {
    }

    // Key, Object Parent
    SettingData(const std::string &_key, Setting<Object> *_parent)
        : ISettingData(_key, _parent)
        , data()
    {
    }

    // Index, Array Parent
    SettingData(unsigned _index, Setting<Array> *_parent)
        : ISettingData(_index, _parent)
        , data()
    {
    }

    void
    setValue(const Type &newValue)
    {
        this->data = newValue;

        this->filled = true;

        if (this->jsonValue != nullptr) {
            JSONWrapper<Type>::setValue(this->jsonValue, newValue);
        }
    }

    Type
    getValue() const
    {
        return this->data;
    }

    Type data;
};

}  // namespace setting
}  // namespace pajlada
