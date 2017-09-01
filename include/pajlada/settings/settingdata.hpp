#pragma once

#include "pajlada/settings/equal.hpp"
#include "pajlada/settings/internal.hpp"
#include "pajlada/settings/serialize.hpp"
#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <boost/any.hpp>
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

    SaveInitialValue = (1ull << 3ull),

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
    ISettingData() = default;

    virtual ~ISettingData() = default;

    SettingOption options = SettingOption::Default;

    inline bool
    optionEnabled(SettingOption option) const
    {
        return (static_cast<uint64_t>(this->options) & static_cast<uint64_t>(option)) != 0;
    }

    void marshal(rapidjson::Document &d);

    virtual rapidjson::Value marshalInto(rapidjson::Document &d) = 0;
    virtual bool unmarshalFrom(rapidjson::Document &d) = 0;

    virtual void registerDocument(rapidjson::Document &d) = 0;

    const std::string &getPath() const;

    void setPath(const std::string &_path);

protected:
    // Setting path (i.e. /a/b/c/3/d/e)
    std::string path;

    rapidjson::Value *get(rapidjson::Document &document);
};

template <typename Type>
class SettingData : public ISettingData, public std::enable_shared_from_this<SettingData<Type>>
{
    SettingData()
        : ISettingData()
        , defaultValue(Type())
        , value(Type())
    {
    }

    SettingData(const Type &_defaultValue)
        : ISettingData()
        , defaultValue(_defaultValue)
        , value(_defaultValue)
    {
    }

    SettingData(Type &&_defaultValue)
        : ISettingData()
        , defaultValue(_defaultValue)
        , value(_defaultValue)
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
        auto valuePointer = this->get(document);
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

        this->valueChanged.connect([this, &d](const Type &, const auto &) {
            this->marshal(d);  //
        });

        if (this->optionEnabled(SettingOption::SaveInitialValue)) {
            this->marshal(d);
        }
    }

    void
    setValue(const Type &newValue, boost::any userData = boost::any())
    {
        if (IsEqual<Type>::get(this->value, newValue)) {
            return;
        }

        this->value = newValue;

        this->valueChanged.invoke(newValue, userData);
    }

    void
    resetToDefaultValue(boost::any userData)
    {
        this->setValue(this->defaultValue, userData);
    }

    void
    setDefaultValue(const Type &newDefaultValue)
    {
        this->defaultValue = newDefaultValue;
    }

    Type
    getDefaultValue() const
    {
        return this->defaultValue;
    }

    Type
    getValue() const
    {
        return this->value;
    }

    Signals::Signal<const Type &, const boost::any &> valueChanged;

private:
    Type defaultValue;
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
