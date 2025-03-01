#pragma once

#include <gtest/gtest.h>

#include <pajlada/settings.hpp>
#include <string>

std::string ReadFile(const std::string &path);

bool FilesMatch(const std::string &fileName1, const std::string &fileName2);

bool LoadFile(const std::string &fileName,
              pajlada::Settings::SettingManager *sm = nullptr);
pajlada::Settings::SettingManager::SaveResult SaveFile(
    const std::string &fileName,
    pajlada::Settings::SettingManager *sm = nullptr);

bool RemoveFile(const std::string &path);

#ifdef PAJLADA_SETTINGS_ENABLE_EXCEPTIONS
#define DD_THROWS(x) REQUIRE_THROWS(x)
#define REQUIRE_IF_NOEXCEPT(x, y)
#define REQUIRE_IF_NOEXCEPT2(x, y)
#else
#define DD_THROWS(x) x
#define REQUIRE_IF_NOEXCEPT(x, y) EXPECT_TRUE(x->getValue() == y)
#define REQUIRE_IF_NOEXCEPT2(x, y) EXPECT_TRUE(x == y)
#endif

struct NonComparableStruct {
    bool a;
};

struct ComparableStruct {
    bool a;

    bool
    operator==(const ComparableStruct &other) const
    {
        return this->a == other.a;
    }
};

namespace pajlada {

template <>
struct Serialize<NonComparableStruct> {
    static rapidjson::Value
    get(const NonComparableStruct &value, rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        ret.AddMember(rapidjson::Value("x", a).Move(),
                      pajlada::Serialize<bool>::get(value.a, a), a);

        return ret;
    }
};

template <>
struct Deserialize<NonComparableStruct> {
    static NonComparableStruct
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        NonComparableStruct ret{
            .a = false,
        };

        if (!value.IsObject()) {
            PAJLADA_REPORT_ERROR(error)
            return ret;
        }

        if (value.HasMember("x")) {
            bool innerError = false;
            auto out = pajlada::Deserialize<bool>::get(value["x"], &innerError);

            if (!innerError) {
                ret.a = out;
            }
        }

        return ret;
    }
};

template <>
struct Serialize<ComparableStruct> {
    static rapidjson::Value
    get(const ComparableStruct &value, rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        ret.AddMember(rapidjson::Value("x", a).Move(),
                      pajlada::Serialize<bool>::get(value.a, a), a);

        return ret;
    }
};

template <>
struct Deserialize<ComparableStruct> {
    static ComparableStruct
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        ComparableStruct ret{
            .a = false,
        };

        if (!value.IsObject()) {
            PAJLADA_REPORT_ERROR(error)
            return ret;
        }

        if (value.HasMember("x")) {
            bool innerError = false;
            auto out = pajlada::Deserialize<bool>::get(value["x"], &innerError);

            if (!innerError) {
                ret.a = out;
            }
        }

        return ret;
    }
};

}  // namespace pajlada
