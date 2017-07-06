#pragma once

#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>

#include <stdexcept>
#include <typeinfo>
#include <vector>

namespace pajlada {
namespace Settings {

// Create a rapidjson::Value from the templated value
template <typename Type>
struct Serialize {
    static rapidjson::Value
    get(const Type &value, rapidjson::Document::AllocatorType &)
    {
        rapidjson::Value ret(value);

        return ret;
    }
};

template <>
struct Serialize<std::string> {
    static rapidjson::Value
    get(const std::string &value, rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(value.c_str(), a);

        return ret;
    }
};

template <>
struct Serialize<Object> {
    static rapidjson::Value
    get(const Object &, rapidjson::Document::AllocatorType &)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        return ret;
    }
};

template <>
struct Serialize<Array> {
    static rapidjson::Value
    get(const Array &, rapidjson::Document::AllocatorType &)
    {
        rapidjson::Value ret(rapidjson::kArrayType);

        return ret;
    }
};

template <typename ContainerType>
struct Serialize<std::vector<ContainerType>> {
    static rapidjson::Value
    get(const std::vector<ContainerType> &value,
        rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kArrayType);

        for (const ContainerType &innerValue : value) {
            ret.PushBack(Serialize<ContainerType>::get(innerValue, a), a);
        }

        return ret;
    }
};

template <typename Type>
struct Deserialize {
    static Type
    get(const rapidjson::Value &value)
    {
        printf("%s\n", typeid(Type).name());
        throw std::runtime_error("Unimplemented deserialize for type");
    }
};

template <>
struct Deserialize<int> {
    static int
    get(const rapidjson::Value &value)
    {
        if (!value.IsInt()) {
            throw std::runtime_error(
                "Deserialized rapidjson::Value is not an int");
        }

        return value.GetInt();
    }
};

template <>
struct Deserialize<bool> {
    static bool
    get(const rapidjson::Value &value)
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
struct Deserialize<double> {
    static double
    get(const rapidjson::Value &value)
    {
        if (!value.IsFloat() && !value.IsDouble() && !value.IsNumber()) {
            throw std::runtime_error(
                "Deserialized rapidjson::Value is not a double");
        }

        return value.GetDouble();
    }
};

template <>
struct Deserialize<float> {
    static float
    get(const rapidjson::Value &value)
    {
        if (!value.IsFloat() && !value.IsDouble() && !value.IsNumber()) {
            throw std::runtime_error(
                "Deserialized rapidjson::Value is not a float");
        }

        return value.GetFloat();
    }
};

template <>
struct Deserialize<std::string> {
    static std::string
    get(const rapidjson::Value &value)
    {
        if (!value.IsString()) {
            throw std::runtime_error(
                "Deserialized rapidjson::Value is not a string");
        }

        return value.GetString();
    }
};

template <>
struct Deserialize<Object> {
    static Object
    get(const rapidjson::Value &)
    {
        // Do nothing
        return Object{};
    }
};

template <>
struct Deserialize<Array> {
    static Array
    get(const rapidjson::Value &)
    {
        // Do nothing
        return Array{};
    }
};

template <typename ContainerType>
struct Deserialize<std::vector<ContainerType>> {
    static std::vector<ContainerType>
    get(const rapidjson::Value &value)
    {
        std::vector<ContainerType> ret;

        if (!value.IsArray()) {
            return ret;
        }

        for (const auto &v : value.GetArray()) {
            ret.push_back(Deserialize<ContainerType>::get(v));
        }

        return ret;
    }
};

}  // namespace Settings
}  // namespace pajlada
