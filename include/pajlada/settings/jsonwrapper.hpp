#pragma once

#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>

#include <memory>
#include <string>

namespace pajlada {
namespace settings {

template <typename Type>
class SettingData;

template <class JSONType>
struct JSONWrapper {
    static rapidjson::Value create(
        const std::shared_ptr<SettingData<JSONType>> &);
};

}  // namespace setting
}  // namespace pajlada
