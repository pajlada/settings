#pragma once

#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>

#include <iostream>
#include <memory>
#include <string>

namespace pajlada {
namespace settings {

template <typename Type>
class SettingData;

template <class JSONType>
struct JSONWrapper {
    static rapidjson::Value
    create(const std::shared_ptr<SettingData<JSONType>> &);

    static void setValue(rapidjson::Value *jsonValue, const JSONType &newValue);
};

}  // namespace setting
}  // namespace pajlada
