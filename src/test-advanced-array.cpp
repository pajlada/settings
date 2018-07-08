#include "libs/catch.hpp"
#include "testhelpers.hpp"

#include <pajlada/settings.hpp>
#include <pajlada/settings/rapidjson-helpers.hpp>

using namespace pajlada::Settings;
using namespace std;

class MyWindow
{
public:
    int x = 0;

    bool
    operator==(const MyWindow &rhs) const
    {
        return std::tie(this->x) == std::tie(rhs.x);
    }
};

TEST_CASE("Single window", "[settings][advanced]")
{
    SettingManager::clear();
    Setting<MyWindow> myWindow("/a");

    REQUIRE(myWindow->x == 0);

    REQUIRE(LoadFile("advanced-window.json"));

    REQUIRE(myWindow->x == 5);
}

TEST_CASE("Multi window", "[settings][advanced]")
{
    SettingManager::clear();
    Setting<std::vector<MyWindow>> myWindows("/a");

    REQUIRE(myWindows->size() == 0);

    REQUIRE(LoadFile("advanced-window-multi.json"));

    REQUIRE(myWindows->size() == 3);

    std::deque<int> values{5, 10, 15};

    for (const auto &window : myWindows.getArray()) {
        REQUIRE(window.x == values.front());
        values.pop_front();
    }

    myWindows.push_back(MyWindow{});

    REQUIRE(myWindows->size() == 4);

    myWindows.push_back(MyWindow{});

    REQUIRE(myWindows->size() == 5);
}

TEST_CASE("Multi files", "[settings][advanced]")
{
    auto sm1 = std::make_shared<SettingManager>();
    auto sm2 = std::make_shared<SettingManager>();
    Setting<std::vector<MyWindow>> myWindows("/a", SettingOption::Default, sm1);

    REQUIRE(myWindows->size() == 0);

    REQUIRE(sm1->load("files/advanced-window-multi.json") ==
            SettingManager::LoadError::NoError);
    REQUIRE(sm2->load("files/advanced-window-multi.json") ==
            SettingManager::LoadError::NoError);

    REQUIRE(myWindows->size() == 3);

    std::deque<int> values{5, 10, 15};

    for (const auto &window : myWindows.getArray()) {
        REQUIRE(window.x == values.front());
        values.pop_front();
    }

    myWindows.push_back(MyWindow{});

    REQUIRE(myWindows->size() == 4);

    myWindows.push_back(MyWindow{});

    REQUIRE(myWindows->size() == 5);

    Setting<std::vector<MyWindow>> myWindows2("/a", SettingOption::Default,
                                              sm2);

    MyWindow test;
    test.x = 77;

    myWindows2.push_back(std::move(test));

    sm1->setPath("lol.json");

    sm2->setPath("lol2.json");
}

namespace pajlada {
namespace Settings {

template <>
struct IsEqual<MyWindow> {
    static bool
    get(const MyWindow &lhs, const MyWindow &rhs)
    {
        return lhs == rhs;
    }
};

template <>
struct Serialize<MyWindow> {
    static rapidjson::Value
    get(const MyWindow &value, rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        rj::set(ret, "x", value.x, a);

        return ret;
    }
};

template <>
struct Deserialize<MyWindow> {
    static MyWindow
    get(const rapidjson::Value &value, bool *error = nullptr)
    {
        MyWindow ret;

        if (!value.IsObject()) {
            PAJLADA_REPORT_ERROR(error)
            PAJLADA_THROW_EXCEPTION("not object")
            return ret;
        }

        rj::get(value, "x", ret.x);

        return ret;
    }
};

}  // namespace Settings
}  // namespace pajlada
