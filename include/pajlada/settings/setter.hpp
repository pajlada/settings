#pragma once

#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

namespace pajlada {
namespace Settings {

template <typename Type>
struct Setter {
    static void
    set(const char *path, const Type &value, rapidjson::Document &document)
    {
        rapidjson::Pointer(path).Set(document, value);
    }
};

template <>
struct Setter<std::string> {
    static void
    set(const char *path, const std::string &value,
        rapidjson::Document &document)
    {
        rapidjson::Pointer(path).Set(document, value.c_str());
    }
};

template <>
struct Setter<Object> {
    static void
    set(const char *path, const Object &, rapidjson::Document &document)
    {
        rapidjson::Pointer(path).Create(document);
    }
};

template <>
struct Setter<Array> {
    static void
    set(const char *path, const Array &, rapidjson::Document &document)
    {
        rapidjson::Pointer(path).Create(document);
    }
};

// Only set the value if it doesn't already exist
template <typename Type>
struct SoftSetter {
    static void
    set(const char *path, const Type &value, rapidjson::Document &document)
    {
        if (rapidjson::Pointer(path).Get(document) == nullptr) {
            Setter<Type>::set(path, value, document);
        }
    }
};

template <>
struct SoftSetter<Object> {
    static void
    set(const char *, const Object &, rapidjson::Document &)
    {
        // Do nothing
    }
};

template <>
struct SoftSetter<Array> {
    static void
    set(const char *, const Array &, rapidjson::Document &)
    {
        // Do nothing
    }
};

}  // namespace Settings
}  // namespace pajlada
