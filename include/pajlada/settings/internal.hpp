#pragma once

#ifdef PAJLADA_SETTINGS_LOG_VERBOSE

#include <rapidjson/document.h>      // IWYU pragma: keep
#include <rapidjson/prettywriter.h>  // IWYU pragma: keep
#include <rapidjson/writer.h>        // IWYU pragma: keep

#include <iostream>  // IWYU pragma: keep

#endif

namespace pajlada::Settings::internal {

#ifdef PAJLADA_SETTINGS_LOG_VERBOSE

inline std::string
pp(const rapidjson::Value &v)
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);
    return {buffer.GetString()};
}

#define PS_DEBUG(x) std::cout << x << '\n';
#else
#define PS_DEBUG(x)
#endif

}  // namespace pajlada::Settings::internal
