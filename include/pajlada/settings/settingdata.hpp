#pragma once

#include "pajlada/settings/internal.hpp"
#include "pajlada/settings/serialize.hpp"
#include "pajlada/settings/setter.hpp"
#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <pajlada/signals/signal.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace pajlada {
namespace Settings {

enum class SettingOption : uint64_t {
    DoNotWriteToJSON = (1ull << 1ull),

    ForceSetOptions = (1ull << 2ull),

    SaveOnChange = (1ull << 3ull),

    SaveInitialValue = (1ull << 4ull),

    Default = 0,
};

inline SettingOption
operator|(const SettingOption &lhs, const SettingOption &rhs)
{
    return (SettingOption)((uint64_t)lhs | (uint64_t)rhs);
}

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
            PS_DEBUG(
                "[" << this->path
                    << "] Skipping marshal due to `DoNotWriteToJSON` setting");
            return;
        }

        PS_DEBUG("[" << this->path << "] Marshalling into document");

        rapidjson::Value v = this->marshalInto(d);

        rapidjson::Pointer(this->getPath().c_str()).Set(d, v);

        this->needsMarshalling = false;
    }

    virtual rapidjson::Value marshalInto(rapidjson::Document &d) = 0;
    virtual bool unmarshalFrom(rapidjson::Document &d) = 0;

    virtual void registerDocument(rapidjson::Document &d) = 0;

    inline uint64_t
    getConnectionID() const
    {
        return this->connectionID;
    }

    const std::string &getPath() const;

    void setPath(const std::string &_path);

    std::atomic<bool> needsMarshalling = {false};

protected:
    // Setting path (i.e. /a/b/c/3/d/e)
    std::string path;

    uint64_t connectionID = 0;

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
        return Serialize<Type>::get(this->getValue(), d.GetAllocator());
    }

    virtual bool
    unmarshalFrom(rapidjson::Document &document) override
    {
        auto valuePointer =
            rapidjson::Pointer(this->getPath().c_str()).Get(document);
        if (valuePointer == nullptr) {
            return false;
        }

        auto newValue = Deserialize<Type>::get(*valuePointer);

        this->setValue(newValue);

        return true;
    }

    virtual void
    registerDocument(rapidjson::Document &d) override
    {
        // PS_DEBUG("[" << this->path << "] Register document");

        this->valueChanged.connect([this, &d](const Type &) {
            if (this->optionEnabled(SettingOption::SaveOnChange)) {
                this->marshal(d);
            } else {
                this->needsMarshalling = true;
            }
        });

        if (this->optionEnabled(SettingOption::SaveInitialValue)) {
            this->marshal(d);
        }
    }

    void
    setValue(const Type &newValue)
    {
        /*
         * TODO(pajlada): Implement templated IsEqual method.
         * That templated thing should by default call the == operator, but can
         * return different values for i.e. map
        if (this->value == newValue) {
            return;
        }
        */

        this->value = newValue;

        this->valueChanged.invoke(newValue);
    }

    Type
    getValue() const
    {
        return this->value;
    }

    Signals::Signal<const Type &> valueChanged;

private:
    Type value;

    Type *
    getValuePointer()
    {
        return &this->value;
    }

    friend class SettingManager;

    template <typename a1, typename a2>
    friend class BorrowedSetting;
};

}  // namespace Settings
}  // namespace pajlada
