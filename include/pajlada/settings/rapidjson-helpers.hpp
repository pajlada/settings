#pragma once

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <pajlada/serialize.hpp>

#include <cassert>
#include <string>

namespace rj {

void
addMember(rapidjson::Value &obj, const char *key, rapidjson::Value &&value,
          rapidjson::Document::AllocatorType &a)
{
    obj.AddMember(rapidjson::Value(key, a).Move(), value, a);
}

void
addMember(rapidjson::Value &obj, const char *key, rapidjson::Value &value,
          rapidjson::Document::AllocatorType &a)
{
    obj.AddMember(rapidjson::Value(key, a).Move(), value.Move(), a);
}

template <typename Type>
void
set(rapidjson::Value &obj, const char *key, const Type &value,
    rapidjson::Document::AllocatorType &a)
{
    assert(obj.IsObject());

    addMember(obj, key, pajlada::Settings::Serialize<Type>::get(value, a), a);
}

template <>
inline void
set(rapidjson::Value &obj, const char *key, const rapidjson::Value &value,
    rapidjson::Document::AllocatorType &a)
{
    assert(obj.IsObject());

    addMember(obj, key, const_cast<rapidjson::Value &>(value), a);
}

template <typename Type>
void
set(rapidjson::Document &obj, const char *key, const Type &value)
{
    assert(obj.IsObject());

    auto &a = obj.GetAllocator();

    addMember(obj, key, pajlada::Settings::Serialize<Type>::get(value, a), a);
}

template <>
inline void
set(rapidjson::Document &obj, const char *key, const rapidjson::Value &value)
{
    assert(obj.IsObject());

    auto &a = obj.GetAllocator();

    addMember(obj, key, const_cast<rapidjson::Value &>(value), a);
}

template <typename Type>
void
add(rapidjson::Value &arr, const Type &value,
    rapidjson::Document::AllocatorType &a)
{
    assert(arr.IsArray());

    arr.PushBack(pajlada::Settings::Serialize<Type>::get(value, a), a);
}

template <typename Type>
bool
get(const rapidjson::Value &obj, const char *key, Type &out)
{
    if (!obj.IsObject()) {
        return false;
    }

    if (!obj.HasMember(key)) {
        return false;
    }

    if (obj.IsNull()) {
        return false;
    }

    bool error = false;
    out = pajlada::Settings::Deserialize<Type>::get(obj[key], &error);

    return !error;
}

template <typename Type>
bool
getSafe(const rapidjson::Value &obj, const char *key, Type &out)
{
    if (!obj.IsObject()) {
        return false;
    }

    if (!obj.HasMember(key)) {
        return false;
    }

    if (obj.IsNull()) {
        return false;
    }

    bool error = false;
    out = pajlada::Settings::Deserialize<Type>::get(obj[key], &error);

    return !error;
}

template <typename Type>
bool
getSafe(const rapidjson::Value &value, Type &out)
{
    bool error = false;
    out = pajlada::Settings::Deserialize<Type>::get(value, &error);

    return !error;
}

static std::string
stringify(const rapidjson::Value &value)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);

    return std::string(buffer.GetString());
}

}  // namespace rj
