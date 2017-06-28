#pragma once

#include "pajlada/settings/serialize.hpp"
#include "pajlada/settings/setter.hpp"
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

enum class SettingOption : uint64_t {
    DoNotWriteToJSON = (1ull << 1ull),

    ForceSetOptions = (1ull << 2ull),

    Default = 0,
};

class ISettingData
{
public:
    ISettingData();

    virtual ~ISettingData() = default;

    SettingOption options = SettingOption::Default;

    inline bool
    optionEnabled(SettingOption option) const
    {
        return (static_cast<uint64_t>(this->options) &
                static_cast<uint64_t>(option)) != 0;
    }

    void
    marshal(rapidjson::Document &d)
    {
        if (this->optionEnabled(SettingOption::DoNotWriteToJSON)) {
            // Don't marshal anything into the setting document
            return;
        }

        rapidjson::Value v = this->marshalInto(d);

        rapidjson::Pointer(this->getPath().c_str()).Set(d, v);
    }

    virtual rapidjson::Value marshalInto(rapidjson::Document &d) = 0;
    virtual bool unmarshalFrom(rapidjson::Document &d) = 0;

    virtual void registerDocument(rapidjson::Document &d) = 0;

    inline uint64_t
    getConnectionID() const
    {
        return this->connectionID;
    }

    inline bool
    isFilled() const
    {
        return this->filled;
    }

    const std::string &getPath() const;

    void setPath(const std::string &_path);

    std::atomic<bool> dirty = {false};

    bool doMarshal = true;

protected:
    // Setting path (i.e. /a/b/c/3/d/e)
    std::string path;

    uint64_t connectionID = 0;

    // If the setting has been filled with any value other than the default
    // one
    bool filled = false;

    rapidjson::Value *
    getValueWithSuffix(const std::string &suffix, rapidjson::Document &document)
    {
        std::string newPath = this->getPath() + suffix;

        return rapidjson::Pointer(newPath.c_str()).Get(document);
    }
};

template <typename Type>
class SettingData : public ISettingData,
                    public std::enable_shared_from_this<SettingData<Type>>
{
    SettingData()
        : ISettingData()
        , value(Type())
    {
    }

    SettingData(const Type &defaultValue)
        : ISettingData()
        , value(defaultValue)
    {
    }

    SettingData(Type &&defaultValue)
        : ISettingData()
        , value(defaultValue)
    {
    }

public:
    virtual rapidjson::Value
    marshalInto(rapidjson::Document &d) override
    {
        return serializeToJSON<Type>::serialize(this->getValue(),
                                                d.GetAllocator());
    }

    virtual bool
    unmarshalFrom(rapidjson::Document &document) override
    {
        auto valuePointer =
            rapidjson::Pointer(this->getPath().c_str()).Get(document);
        if (valuePointer == nullptr) {
            return false;
        }

        auto newValue = deserializeJSON<Type>::deserialize(*valuePointer);

        this->setValue(newValue);

        return true;
    }

    virtual void
    registerDocument(rapidjson::Document &d) override
    {
        this->valueChanged.connect([this /*, &d*/](const Type &) {
            // for now, just set as dirty
            this->dirty = true;
            // this->marshalInto(d);  //
        });
    }

    void
    setValue(const Type &newValue)
    {
        if (this->value == newValue) {
            return;
        }

        this->value = newValue;

        this->filled = true;

        this->valueChanged.invoke(newValue);
    }

    Type
    getValue() const
    {
        return this->value;
    }

    const Type &
    getValueRef() const
    {
        return this->value;
    }

    Signals::Signal<const Type &> valueChanged;

private:
    Type value;

    friend class SettingManager;
};

}  // namespace Settings
}  // namespace pajlada
