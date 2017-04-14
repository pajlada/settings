#pragma once

#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>

#include <memory>

namespace pajlada {
namespace settings {
namespace detail {

template <typename Type>
class SettingData;

template <class JSONType>
struct JSONWrapper {
    static rapidjson::Value create(
        const std::shared_ptr<SettingData<JSONType>> &);
};

}  // namespace detail
}  // namespace settings
}  // namespace pajlada
