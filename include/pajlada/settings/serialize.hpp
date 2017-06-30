#pragma once

#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>

#include <stdexcept>
#include <typeinfo>

namespace pajlada {
namespace Settings {

// Create a rapidjson::Value from the templated value
template <typename Type>
struct serializeToJSON {
    static rapidjson::Value
    serialize(const Type &value, rapidjson::Document::AllocatorType &)
    {
        rapidjson::Value ret(value);

        return ret;
    }
};

template <>
struct serializeToJSON<std::string> {
    static rapidjson::Value
    serialize(const std::string &value, rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(value.c_str(), a);

        return ret;
    }
};

template <>
struct serializeToJSON<Object> {
    static rapidjson::Value
    serialize(const Object &, rapidjson::Document::AllocatorType &)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        return ret;
    }
};

template <>
struct serializeToJSON<Array> {
    static rapidjson::Value
    serialize(const Array &, rapidjson::Document::AllocatorType &)
    {
        rapidjson::Value ret(rapidjson::kArrayType);

        return ret;
    }
};

template <typename Type>
struct deserializeJSON {
    static Type
    deserialize(const rapidjson::Value &value)
    {
        printf("%s\n", typeid(Type).name());
        throw std::runtime_error("Unimplemented deserialize for type");
    }
};

template <>
struct deserializeJSON<int> {
    static int
    deserialize(const rapidjson::Value &value)
    {
        if (!value.IsInt()) {
            throw std::runtime_error(
                "Deserialized rapidjson::Value is not an int");
        }

        return value.GetInt();
    }
};

template <>
struct deserializeJSON<bool> {
    static bool
    deserialize(const rapidjson::Value &value)
    {
        if (value.IsBool()) {
            // No conversion needed
            return value.GetBool();
        }

        if (value.IsInt()) {
            // Conversion from Int:
            // 1 == true
            // Anything else = false
            return value.GetInt() == 1;
        }

        throw std::runtime_error("Deserialized rapidjson::Value is not an int");
    }
};

template <>
struct deserializeJSON<double> {
    static double
    deserialize(const rapidjson::Value &value)
    {
        if (!value.IsFloat() && !value.IsDouble() && !value.IsNumber()) {
            throw std::runtime_error(
                "Deserialized rapidjson::Value is not a double");
        }

        return value.GetDouble();
    }
};

template <>
struct deserializeJSON<float> {
    static float
    deserialize(const rapidjson::Value &value)
    {
        if (!value.IsFloat() && !value.IsDouble() && !value.IsNumber()) {
            throw std::runtime_error(
                "Deserialized rapidjson::Value is not a float");
        }

        return value.GetFloat();
    }
};

template <>
struct deserializeJSON<std::string> {
    static std::string
    deserialize(const rapidjson::Value &value)
    {
        if (!value.IsString()) {
            throw std::runtime_error(
                "Deserialized rapidjson::Value is not a string");
        }

        return value.GetString();
    }
};

template <>
struct deserializeJSON<Object> {
    static Object
    deserialize(const rapidjson::Value &)
    {
        // Do nothing
        return Object{};
    }
};

template <>
struct deserializeJSON<Array> {
    static Array
    deserialize(const rapidjson::Value &)
    {
        // Do nothing
        return Array{};
    }
};

}  // namespace Settings
}  // namespace pajlada
