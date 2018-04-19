#pragma once

#include <iostream>

namespace pajlada {
namespace Settings {

class Exception
{
public:
    enum ErrorCode {
        ExpiredSetting,
    } code;

    Exception() = delete;

    explicit Exception(ErrorCode &&_code) noexcept
        : code(_code)
    {
    }

    ~Exception() noexcept = default;
};

}  // namespace Settings
}  // namespace pajlada
