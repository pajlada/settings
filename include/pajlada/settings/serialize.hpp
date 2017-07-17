#pragma once

#include "pajlada/settings/types.hpp"

#include <rapidjson/document.h>
#include <boost/any.hpp>

#include <cassert>
#include <map>
#include <stdexcept>
#include <typeinfo>
#include <vector>

namespace pajlada {
namespace Settings {

boost::any DeserializeAny(const rapidjson::Value &value);

template <typename Type>
void AddMember(rapidjson::Value &object, const char *key, const Type &value,
               rapidjson::Document::AllocatorType &a);

template <typename Type>
void PushBack(rapidjson::Value &array, const Type &value,
              rapidjson::Document::AllocatorType &a);

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

template <>
struct Serialize<std::map<std::string, boost::any>> {
    static rapidjson::Value
    get(const std::map<std::string, boost::any> &value,
        rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        for (auto it = value.begin(); it != value.end(); ++it) {
            AddMember(ret, it->first.c_str(), it->second, a);
        }

        return ret;
    }
};

template <>
struct Serialize<std::vector<boost::any>> {
    static rapidjson::Value
    get(const std::vector<boost::any> &value,
        rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kArrayType);

        for (const auto &innerValue : value) {
            PushBack(ret, innerValue, a);
        }

        return ret;
    }
};

template <>
struct Serialize<boost::any> {
    static rapidjson::Value
    get(const boost::any &value, rapidjson::Document::AllocatorType &a)
    {
        using boost::any_cast;

        if (value.empty()) {
            return rapidjson::Value(rapidjson::kNullType);
        }

        if (value.type() == typeid(int)) {
            return Serialize<int>::get(any_cast<int>(value), a);
        } else if (value.type() == typeid(float)) {
            return Serialize<float>::get(any_cast<float>(value), a);
        } else if (value.type() == typeid(double)) {
            return Serialize<double>::get(any_cast<double>(value), a);
        } else if (value.type() == typeid(bool)) {
            return Serialize<bool>::get(any_cast<bool>(value), a);
        } else if (value.type() == typeid(std::string)) {
            return Serialize<std::string>::get(any_cast<std::string>(value), a);
        } else if (value.type() == typeid(std::map<std::string, boost::any>)) {
            return Serialize<std::map<std::string, boost::any>>::get(
                any_cast<std::map<std::string, boost::any>>(value), a);
        } else if (value.type() == typeid(std::vector<boost::any>)) {
            return Serialize<std::vector<boost::any>>::get(
                any_cast<std::vector<boost::any>>(value), a);
        } else {
            PS_DEBUG("[boost::any] Deserialize: Unknown type of value");
        }

        return rapidjson::Value(rapidjson::kNullType);
    }
};

///////////////////////////////////////////////////////////////////////////////

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

template <>
struct Deserialize<boost::any> {
    static boost::any get(const rapidjson::Value &value);
};

template <>
struct Deserialize<std::map<std::string, boost::any>> {
    static std::map<std::string, boost::any>
    get(const rapidjson::Value &value)
    {
        std::map<std::string, boost::any> ret;

        if (!value.IsObject()) {
            PS_DEBUG("[std::map<std::string, boost::any>] "
                     "Deserialize: Value "
                     "is not a map");
            return ret;
        }

        for (rapidjson::Value::ConstMemberIterator it = value.MemberBegin();
             it != value.MemberEnd(); ++it) {
            ret.emplace(it->name.GetString(),
                        Deserialize<boost::any>::get(it->value));
        }

        return ret;
    }
};

template <>
struct Deserialize<std::vector<boost::any>> {
    static std::vector<boost::any>
    get(const rapidjson::Value &value)
    {
        std::vector<boost::any> ret;

        if (!value.IsArray()) {
            PS_DEBUG("[std::vector<boost::any>] "
                     "Deserialize: Value "
                     "is not an array");
            return ret;
        }

        for (const rapidjson::Value &innerValue : value.GetArray()) {
            ret.emplace_back(Deserialize<boost::any>::get(innerValue));
        }

        return ret;
    }
};

inline boost::any
Deserialize<boost::any>::get(const rapidjson::Value &value)
{
    if (value.IsInt()) {
        return value.GetInt();
    } else if (value.IsFloat() || value.IsDouble()) {
        return value.GetDouble();
    } else if (value.IsString()) {
        return std::string(value.GetString());
    } else if (value.IsBool()) {
        return value.GetBool();
    } else if (value.IsObject()) {
        return Deserialize<std::map<std::string, boost::any>>::get(value);
    } else if (value.IsArray()) {
        return Deserialize<std::vector<boost::any>>::get(value);
    }

    return boost::any();
}

template <typename Type>
inline void
AddMember(rapidjson::Value &object, const char *key, const Type &value,
          rapidjson::Document::AllocatorType &a)
{
    assert(object.IsObject());

    object.AddMember(rapidjson::Value(key, a).Move(),
                     Serialize<Type>::get(value, a), a);
}

template <typename Type>
inline void
PushBack(rapidjson::Value &array, const Type &value,
         rapidjson::Document::AllocatorType &a)
{
    assert(array.IsArray());

    array.PushBack(Serialize<Type>::get(value, a), a);
}

}  // namespace Settings
}  // namespace pajlada
