#pragma once

#include <pajlada/settings/internal.hpp>

#include <rapidjson/document.h>

#ifndef PAJLADA_SETTINGS_NO_BOOST
#include <boost/any.hpp>
#endif

#define PAJLADA_SETTINGS_ROUNDING_METHOD_ROUND 0
#define PAJLADA_SETTINGS_ROUNDING_METHOD_CEIL 1
#define PAJLADA_SETTINGS_ROUNDING_METHOD_FLOOR 2

// valid values: ceil/floor/round
#ifndef PAJLADA_SETTINGS_ROUNDING_METHOD
#define PAJLADA_SETTINGS_ROUNDING_METHOD PAJLADA_SETTINGS_ROUNDING_METHOD_ROUND
#endif

#ifdef PAJLADA_SETTINGS_ENABLE_EXCEPTIONS
#define PAJLADA_THROW_EXCEPTION(x) throw std::runtime_error(x);
#else
#define PAJLADA_THROW_EXCEPTION(x)
#endif

#ifndef PAJLADA_REPORT_ERROR
#define PAJLADA_REPORT_ERROR(x) \
    if (x != nullptr) {         \
        *x = true;              \
    }
#endif

#include <cassert>
#include <cmath>
#include <map>
#include <stdexcept>
#include <typeinfo>
#include <vector>

namespace pajlada {
namespace Settings {

namespace {

template <typename T1, typename T2>
typename std::enable_if<std::is_integral<T1>::value, T1>::type
Round(T2 value)
{
#if PAJLADA_SETTINGS_ROUNDING_METHOD == PAJLADA_SETTINGS_ROUNDING_METHOD_ROUND
    return round(value);
#elif PAJLADA_SETTINGS_ROUNDING_METHOD == PAJLADA_SETTINGS_ROUNDING_METHOD_CEIL
    return ceil(value);
#elif PAJLADA_SETTINGS_ROUNDING_METHOD == PAJLADA_SETTINGS_ROUNDING_METHOD_FLOOR
    return floor(value);
#else
    static_assert(
        "Invalid rounding method selected in PAJLADA_SETTINGS_ROUNDING_METHOD");
#endif
}

template <typename Type, typename std::enable_if<
                             std::is_integral<Type>::value>::type * = nullptr>
Type
GetNumber(const rapidjson::Value &value)
{
    if (value.IsDouble()) {
        return Round<Type>(value.GetDouble());
    }
    if (value.IsFloat()) {
        return Round<Type>(value.GetFloat());
    }
    if (value.IsInt()) {
        return value.GetInt();
    }

    return Type{};
}

}  // namespace

// Serialize is called when a settings value is being saved
// Deserialize is called when we load a json file into our library

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

template <typename Arg1, typename Arg2>
struct Serialize<std::pair<Arg1, Arg2>> {
    static rapidjson::Value
    get(const std::pair<Arg1, Arg2> &value,
        rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kArrayType);

        ret.PushBack(Serialize<Arg1>::get(value.first, a), a);
        ret.PushBack(Serialize<Arg2>::get(value.second, a), a);

        return ret;
    }
};

template <typename ValueType>
struct Serialize<std::map<std::string, ValueType>> {
    static rapidjson::Value
    get(const std::map<std::string, ValueType> &value,
        rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        for (auto it = value.begin(); it != value.end(); ++it) {
            AddMember(ret, it->first.c_str(), it->second, a);
        }

        return ret;
    }
};

template <typename ValueType>
struct Serialize<std::vector<ValueType>> {
    static rapidjson::Value
    get(const std::vector<ValueType> &value,
        rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kArrayType);

        for (const auto &innerValue : value) {
            PushBack(ret, innerValue, a);
        }

        return ret;
    }
};

#ifndef PAJLADA_SETTINGS_NO_BOOST
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
        } else if (value.type() == typeid(const char *)) {
            return Serialize<std::string>::get(any_cast<const char *>(value),
                                               a);
        } else if (value.type() == typeid(std::map<std::string, boost::any>)) {
            return Serialize<std::map<std::string, boost::any>>::get(
                any_cast<std::map<std::string, boost::any>>(value), a);
        } else if (value.type() == typeid(std::vector<boost::any>)) {
            return Serialize<std::vector<boost::any>>::get(
                any_cast<std::vector<boost::any>>(value), a);
        } else if (value.type() == typeid(std::vector<std::string>)) {
            return Serialize<std::vector<std::string>>::get(
                any_cast<std::vector<std::string>>(value), a);
        } else {
            PS_DEBUG("[boost::any] Serialize: Unknown type of value");
        }

        return rapidjson::Value(rapidjson::kNullType);
    }
};
#endif

///////////////////////////////////////////////////////////////////////////////

template <typename Type, typename Enable = void>
struct Deserialize {
    static Type
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        // static_assert(false, "Unimplemented deserialize type");

        printf("%s\n", typeid(Type).name());
        throw std::runtime_error("Unimplemented deserialize for type");
        PAJLADA_THROW_EXCEPTION("Unimplemented deserialize for type")
        PAJLADA_REPORT_ERROR(error)
        return Type{};
    }
};

template <typename Type>
struct Deserialize<
    Type, typename std::enable_if<std::is_integral<Type>::value>::type> {
    static Type
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsNumber()) {
            PAJLADA_REPORT_ERROR(error)
            PAJLADA_THROW_EXCEPTION(
                "Invalid json type read for integral deserializer")
            return Type{};
        }

        return GetNumber<Type>(value);
    }
};

template <>
struct Deserialize<bool> {
    static bool
    get(const rapidjson::Value &value, bool *error = nullptr)
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

        PAJLADA_REPORT_ERROR(error)
        PAJLADA_THROW_EXCEPTION("Invalid json type read for bool deserializer")
        return false;
    }
};

template <>
struct Deserialize<double> {
    static double
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsNumber()) {
            PAJLADA_REPORT_ERROR(error)
            PAJLADA_THROW_EXCEPTION(
                "Invalid json type read for double deserializer")
            return double{};
        }

        return value.GetDouble();
    }
};

template <>
struct Deserialize<float> {
    static float
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsNumber()) {
            PAJLADA_REPORT_ERROR(error)
            PAJLADA_THROW_EXCEPTION(
                "Invalid json type read for float deserializer")
            return float{};
        }

        return value.GetFloat();
    }
};

template <>
struct Deserialize<std::string> {
    static std::string
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsString()) {
            PAJLADA_REPORT_ERROR(error)
            PAJLADA_THROW_EXCEPTION(
                "Invalid json type read for string deserializer")
            return std::string{};
        }

        return value.GetString();
    }
};

template <typename ValueType>
struct Deserialize<std::map<std::string, ValueType>> {
    static std::map<std::string, ValueType>
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        std::map<std::string, ValueType> ret;

        if (!value.IsObject()) {
            PS_DEBUG("[std::map<std::string, ValueType>] "
                     "Deserialize: Value "
                     "is not a map");
            return ret;
        }

        for (rapidjson::Value::ConstMemberIterator it = value.MemberBegin();
             it != value.MemberEnd(); ++it) {
            ret.emplace(it->name.GetString(),
                        Deserialize<ValueType>::get(it->value));
        }

        return ret;
    }
};

template <typename ValueType>
struct Deserialize<std::vector<ValueType>> {
    static std::vector<ValueType>
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        std::vector<ValueType> ret;

        if (!value.IsArray()) {
            PS_DEBUG("[std::vector<ValueType>] "
                     "Deserialize: Value "
                     "is not an array");
            return ret;
        }

        for (const rapidjson::Value &innerValue : value.GetArray()) {
            ret.emplace_back(Deserialize<ValueType>::get(innerValue));
        }

        return ret;
    }
};

template <typename Arg1, typename Arg2>
struct Deserialize<std::pair<Arg1, Arg2>> {
    static std::pair<Arg1, Arg2>
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsArray()) {
            PS_DEBUG("[std::pair: Value is not a 2-size array");
            return std::make_pair(Arg1(), Arg2());
        }

        if (value.Size() != 2) {
            PS_DEBUG("[std::pair: Value is not a 2-size array");
            return std::make_pair(Arg1(), Arg2());
        }

        return std::make_pair(Deserialize<Arg1>::get(value[0]),
                              Deserialize<Arg2>::get(value[1]));
    }
};

#ifndef PAJLADA_SETTINGS_NO_BOOST
template <>
struct Deserialize<boost::any> {
    static boost::any
    get(const rapidjson::Value &value, bool *error = nullptr)
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
};
#endif

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
