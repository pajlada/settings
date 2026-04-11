#include <gtest/gtest.h>

#include <pajlada/settings.hpp>
#include <variant>

#include "common.hpp"

using namespace pajlada::Settings;
using SaveResult = pajlada::Settings::SettingManager::SaveResult;
using LoadError = pajlada::Settings::SettingManager::LoadError;

namespace {

class BaseHighlight
{
public:
    bool
    isEnabled() const
    {
        return this->enabled.value_or(true);
    }

    void
    setEnabled(std::optional<bool> newEnabledState)
    {
        this->enabled.swap(newEnabledState);
    }

    void
    serialize(rapidjson::Value &ret,
              rapidjson::Document::AllocatorType &a) const
    {
        if (this->enabled.has_value()) {
            ret.AddMember(
                rapidjson::Value("enabled", a).Move(),
                pajlada::Serialize<bool>::get(this->enabled.value(), a), a);
        }
    }

    bool
    deserialize(const rapidjson::Value &value)
    {
        assert(value.IsObject());

        if (value.HasMember("enabled")) {
            bool innerError = false;
            this->enabled =
                pajlada::Deserialize<bool>::get(value["enabled"], &innerError);
            if (innerError) {
                return false;
            }
        }

        return true;
    }

private:
    std::optional<bool> enabled;
};

class YourUsernameHighlight : public BaseHighlight
{
public:
    static constexpr std::string_view ID = "yourusername";
};

class WhispersHighlight : public BaseHighlight
{
public:
    static constexpr std::string_view ID = "whispers";
};

class SubscriptionsHighlight : public BaseHighlight
{
public:
    static constexpr std::string_view ID = "subscriptions";
};

bool
matchesId(const rapidjson::Value &value, std::string_view expectedID)
{
    auto idIt = value.FindMember("id");
    if (idIt == value.MemberEnd()) {
        // no id field
        return false;
    }

    if (!idIt->value.IsString()) {
        // id field is not a string
        return false;
    }

    if (expectedID != idIt->value.GetString()) {
        // id field does not match expectation
        return false;
    }

    return true;
}

}  // namespace

namespace pajlada {

template <>
struct Serialize<YourUsernameHighlight> {
    static rapidjson::Value
    get(const YourUsernameHighlight &value,
        rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        ret.AddMember(
            rapidjson::Value("id", a).Move(),
            Serialize<std::string_view>::get(YourUsernameHighlight::ID, a), a);

        value.serialize(ret, a);

        return ret;
    }
};

template <>
struct Deserialize<YourUsernameHighlight> {
    static YourUsernameHighlight
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsObject()) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        if (!matchesId(value, YourUsernameHighlight::ID)) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        YourUsernameHighlight ret;

        if (!ret.deserialize(value)) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        return ret;
    }
};

template <>
struct Serialize<WhispersHighlight> {
    static rapidjson::Value
    get(const WhispersHighlight &value, rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        ret.AddMember(
            rapidjson::Value("id", a).Move(),
            Serialize<std::string_view>::get(WhispersHighlight::ID, a), a);

        value.serialize(ret, a);

        return ret;
    }
};

template <>
struct Deserialize<WhispersHighlight> {
    static WhispersHighlight
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsObject()) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        if (!matchesId(value, WhispersHighlight::ID)) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        WhispersHighlight ret;

        if (!ret.deserialize(value)) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        return ret;
    }
};

template <>
struct Serialize<SubscriptionsHighlight> {
    static rapidjson::Value
    get(const SubscriptionsHighlight &value,
        rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        ret.AddMember(
            rapidjson::Value("id", a).Move(),
            Serialize<std::string_view>::get(SubscriptionsHighlight::ID, a), a);

        value.serialize(ret, a);

        return ret;
    }
};

template <>
struct Deserialize<SubscriptionsHighlight> {
    static SubscriptionsHighlight
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsObject()) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        if (!matchesId(value, SubscriptionsHighlight::ID)) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        SubscriptionsHighlight ret;

        if (!ret.deserialize(value)) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        return ret;
    }
};

}  // namespace pajlada

TEST(HighlightsVariant, SerializeVariant)
{
    rapidjson::Document d;
    rapidjson::Value actual;

    std::variant<std::variant<YourUsernameHighlight, WhispersHighlight,
                              SubscriptionsHighlight>>
        v = YourUsernameHighlight{};
    actual = pajlada::Serialize<decltype(v)>::get(v, d.GetAllocator());

    AssertValueMatch(R"({
    "id": "yourusername"
})",
                     actual);
}

TEST(HighlightsVariant, 1)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = static_cast<SettingManager::SaveMethod>(
        static_cast<uint64_t>(SettingManager::SaveMethod::SaveManually) |
        static_cast<uint64_t>(SettingManager::SaveMethod::OnlySaveIfChanged));
    sm->setBackupEnabled(false);

    Setting<std::vector<std::variant<YourUsernameHighlight, WhispersHighlight,
                                     SubscriptionsHighlight>>>
        highlights("/highlights",
                   pajlada::Settings::SettingOption::CompareBeforeSet, sm);

    ASSERT_EQ(SaveResult::Skipped,
              sm->saveAs("files/highlights-variant/out.1.json"));

    highlights.push_back(YourUsernameHighlight{});

    ASSERT_EQ(SaveResult::Success,
              sm->saveAs("files/highlights-variant/out.1.json"));

    AssertFilesMatch("highlights-variant/correct.1.1.json",
                     "highlights-variant/out.1.json");
    ASSERT_EQ(SaveResult::Skipped,
              sm->saveAs("files/highlights-variant/out.1.json"));

    highlights.push_back(WhispersHighlight{});

    ASSERT_EQ(SaveResult::Success,
              sm->saveAs("files/highlights-variant/out.1.json"));

    AssertFilesMatch("highlights-variant/correct.1.2.json",
                     "highlights-variant/out.1.json");
    ASSERT_EQ(SaveResult::Skipped,
              sm->saveAs("files/highlights-variant/out.1.json"));

    highlights.push_back(SubscriptionsHighlight{});

    ASSERT_EQ(SaveResult::Success,
              sm->saveAs("files/highlights-variant/out.1.json"));

    AssertFilesMatch("highlights-variant/correct.1.3.json",
                     "highlights-variant/out.1.json");
    ASSERT_EQ(SaveResult::Skipped,
              sm->saveAs("files/highlights-variant/out.1.json"));

    {
        auto currentHighlights = highlights.getValueCopy();

        ASSERT_FALSE(highlights.setValue(currentHighlights));

        auto &whispersHighlight =
            std::get<WhispersHighlight>(currentHighlights[1]);
        whispersHighlight.setEnabled(false);

        ASSERT_TRUE(highlights.setValue(currentHighlights));
    }

    ASSERT_EQ(SaveResult::Success,
              sm->saveAs("files/highlights-variant/out.1.json"));

    AssertFilesMatch("highlights-variant/correct.1.4.json",
                     "highlights-variant/out.1.json");

    {
        auto currentHighlights = highlights.getValueCopy();

        ASSERT_FALSE(highlights.setValue(currentHighlights));

        auto &whispersHighlight =
            std::get<WhispersHighlight>(currentHighlights[1]);
        whispersHighlight.setEnabled({});

        ASSERT_TRUE(highlights.setValue(currentHighlights));
    }

    ASSERT_EQ(SaveResult::Success,
              sm->saveAs("files/highlights-variant/out.1.json"));

    AssertFilesMatch("highlights-variant/correct.1.5.json",
                     "highlights-variant/out.1.json");
}
