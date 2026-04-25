#include <gtest/gtest.h>

#include <pajlada/settings.hpp>

#include "common.hpp"

using namespace pajlada::Settings;
using SaveResult = pajlada::Settings::SettingManager::SaveResult;
using LoadError = pajlada::Settings::SettingManager::LoadError;

namespace {

struct Foo {
    explicit Foo(const std::shared_ptr<SettingManager> &sm)
        : a("/foo/0/a", 5, sm)
        , b("/foo/0/b", 10, sm)
    {
    }

    Setting<int> a;
    Setting<int> b;
};

}  // namespace

TEST(Complex, Foo)
{
    using std::any_cast;

    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;
    sm->setBackupEnabled(false);

    Foo foo(sm);

    foo.a = 6;

    // foo.a.resetToDefaultValue();

    EXPECT_EQ(SaveResult::Success, sm->saveAs("files/complex/out.foo.json"));

    AssertFilesMatch("complex/correct.foo.json", "complex/out.foo.json");
}

namespace {

class Highlight
{
public:
    Highlight()
        : id("INVALID")
    {
    }

    explicit Highlight(std::string_view id_)
        : id(id_)
    {
    }

    bool operator==(const Highlight &other) const = default;

    friend std::ostream &
    operator<<(std::ostream &os, const Highlight &v)
    {
        os << "(pattern='" << v.getPattern() << "' effect='" << v.getEffect()
           << "')";
        return os;
    }

    void
    setPattern(const std::optional<std::string> &newPattern)
    {
        this->pattern = newPattern;
    }

    std::string
    getPattern() const
    {
        return this->pattern.value_or("default_pattern");
    }

    std::string
    getEffect() const
    {
        return this->effect.value_or("default_effect");
    }

private:
    std::string id;

    std::optional<std::string> pattern;
    std::optional<std::string> effect;

    friend struct pajlada::Serialize<Highlight>;
    friend struct pajlada::Deserialize<Highlight>;
};

}  // namespace

namespace pajlada {

template <>
struct IsEqual<Highlight> {
    static bool
    get(const Highlight &lhs, const Highlight &rhs)
    {
        return lhs == rhs;
    }
};

template <>
struct Serialize<Highlight> {
    static rapidjson::Value
    get(const Highlight &value, rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        ret.AddMember(rapidjson::Value("id", a).Move(),
                      pajlada::Serialize<std::string>::get(value.id, a), a);

        if (value.pattern.has_value()) {
            ret.AddMember(
                rapidjson::Value("pattern", a).Move(),
                pajlada::Serialize<std::string>::get(value.pattern.value(), a),
                a);
        }

        if (value.effect.has_value()) {
            ret.AddMember(
                rapidjson::Value("effect", a).Move(),
                pajlada::Serialize<std::string>::get(value.effect.value(), a),
                a);
        }

        return ret;
    }
};

template <>
struct Deserialize<Highlight> {
    static Highlight
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        if (!value.IsObject()) {
            PAJLADA_REPORT_ERROR(error)
            return {};
        }

        bool innerError = false;

        std::string id;

        if (value.HasMember("id")) {
            id = pajlada::Deserialize<std::string>::get(value["id"],
                                                        &innerError);
            if (innerError) {
                PAJLADA_REPORT_ERROR(error);
                return {};
            }
        }

        Highlight ret(id);

        if (value.HasMember("pattern")) {
            ret.pattern = pajlada::Deserialize<std::string>::get(
                value["pattern"], &innerError);
            if (innerError) {
                PAJLADA_REPORT_ERROR(error);
                return {};
            }
        }

        if (value.HasMember("effect")) {
            bool innerError = false;
            ret.effect = pajlada::Deserialize<std::string>::get(value["effect"],
                                                                &innerError);
            if (innerError) {
                PAJLADA_REPORT_ERROR(error);
                return {};
            }
        }

        return ret;
    }
};

}  // namespace pajlada

TEST(Complex, Highlights)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = static_cast<SettingManager::SaveMethod>(
        static_cast<uint64_t>(SettingManager::SaveMethod::SaveManually) |
        static_cast<uint64_t>(SettingManager::SaveMethod::OnlySaveIfChanged));
    sm->setBackupEnabled(false);

    Setting<std::vector<Highlight>> highlights(
        "/highlights", pajlada::Settings::SettingOption::CompareBeforeSet, sm);

    // EXPECT_EQ(SaveResult::Skipped,
    //           sm->saveAs("files/complex/out.highlights.json"));

    // // Vector is empty, there should be nothing to save
    // AssertFilesMatch("complex/correct.highlights.1.json",
    //                  "complex/out.highlights.json");

    highlights.push_back(Highlight("subscription"));

    ASSERT_EQ(highlights.getValue()[0].getPattern(), "default_pattern");

    std::cout << "highlights: " << highlights.getValue()[0] << '\n';

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/complex/out.highlights.json"));

    // Vector has one entry, id is set and the pattern & effect are default-initialized
    AssertFilesMatch("complex/correct.highlights.2.json",
                     "complex/out.highlights.json");

    {
        auto h = highlights.getValueCopy();
        h[0].setPattern("new pattern");
        ASSERT_TRUE(highlights.setValue(h));
        ASSERT_FALSE(highlights.setValue(h));
    }

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/complex/out.highlights.json"));

    // pattern should be updated
    AssertFilesMatch("complex/correct.highlights.3.json",
                     "complex/out.highlights.json");

    {
        std::vector<Highlight> h{};
        h.emplace_back("subscription");
        h[0].setPattern("new pattern");

        // new vector same value, still comparing as the same
        ASSERT_FALSE(highlights.setValue(h));
    }

    // nothing changed, save should be skipped
    EXPECT_EQ(SaveResult::Skipped,
              sm->saveAs("files/complex/out.highlights.json"));

    {
        std::vector<Highlight> h{
            Highlight{"subscription"},
            Highlight{"mention"},
        };
        h[0].setPattern("subscription pattern");
        h[1].setPattern("mention pattern");

        ASSERT_TRUE(highlights.setValue(h));
    }

    ASSERT_EQ(SaveResult::Success,
              sm->saveAs("files/complex/out.highlights.json"));

    // pattern should be updated
    AssertFilesMatch("complex/correct.highlights.4.json",
                     "complex/out.highlights.json");
}

TEST(Complex, HighlightVariant)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = static_cast<SettingManager::SaveMethod>(
        static_cast<uint64_t>(SettingManager::SaveMethod::SaveManually) |
        static_cast<uint64_t>(SettingManager::SaveMethod::OnlySaveIfChanged));
    sm->setBackupEnabled(false);

    Setting<std::vector<Highlight>> highlights(
        "/highlights", pajlada::Settings::SettingOption::CompareBeforeSet, sm);

    // EXPECT_EQ(SaveResult::Skipped,
    //           sm->saveAs("files/complex/out.highlights.json"));

    // // Vector is empty, there should be nothing to save
    // AssertFilesMatch("complex/correct.highlights.1.json",
    //                  "complex/out.highlights.json");

    highlights.push_back(Highlight("subscription"));

    ASSERT_EQ(highlights.getValue()[0].getPattern(), "default_pattern");

    std::cout << "highlights: " << highlights.getValue()[0] << '\n';

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/complex/out.highlights.json"));

    // Vector has one entry, id is set and the pattern & effect are default-initialized
    AssertFilesMatch("complex/correct.highlights.2.json",
                     "complex/out.highlights.json");

    {
        auto h = highlights.getValueCopy();
        h[0].setPattern("new pattern");
        ASSERT_TRUE(highlights.setValue(h));
        ASSERT_FALSE(highlights.setValue(h));
    }

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/complex/out.highlights.json"));

    // pattern should be updated
    AssertFilesMatch("complex/correct.highlights.3.json",
                     "complex/out.highlights.json");

    {
        std::vector<Highlight> h{};
        h.emplace_back("subscription");
        h[0].setPattern("new pattern");

        // new vector same value, still comparing as the same
        ASSERT_FALSE(highlights.setValue(h));
    }

    // nothing changed, save should be skipped
    EXPECT_EQ(SaveResult::Skipped,
              sm->saveAs("files/complex/out.highlights.json"));

    {
        std::vector<Highlight> h{
            Highlight{"subscription"},
            Highlight{"mention"},
        };
        h[0].setPattern("subscription pattern");
        h[1].setPattern("mention pattern");

        ASSERT_TRUE(highlights.setValue(h));
    }

    ASSERT_EQ(SaveResult::Success,
              sm->saveAs("files/complex/out.highlights.json"));

    // pattern should be updated
    AssertFilesMatch("complex/correct.highlights.4.json",
                     "complex/out.highlights.json");
}
