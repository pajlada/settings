#pragma once

#include "pajlada/settings/settingdata.hpp"

#include <iostream>
#include <memory>

namespace pajlada {
namespace Settings {

template <typename Type, typename Container = SettingData<Type>>
class BorrowedSetting
{
public:
    explicit BorrowedSetting(std::shared_ptr<Container> _ptr)
        : ptr(std::move(_ptr))
        , originalValue(this->ptr->getValue())
    {
    }

    ~BorrowedSetting()
    {
        if (this->originalValue != this->ptr->getValue()) {
            // Note that the change can have occured from outside this borrowed
            // setting
            this->ptr->valueChanged.invoke(this->ptr->getValue());
        }
    }

    Type *operator->()
    {
        return this->ptr.operator->()->getValuePointer();
    }

    BorrowedSetting &
    operator=(Type &&newValue) noexcept
    {
        this->ptr->setValue(std::move(newValue));

        this->originalValue = this->ptr->getValue();

        return *this;
    }

private:
    std::shared_ptr<Container> ptr;

    Type originalValue;
};

}  // namespace Settings
}  // namespace pajlada
