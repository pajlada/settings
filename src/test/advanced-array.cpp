#include <pajlada/settings.hpp>

#include "test/common.hpp"

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

TEST(AdvancedArray, SingleWindow)
{
    SettingManager::clear();
    Setting<MyWindow> myWindow("/a");

    // EXPECT_TRUE(myWindow->x == 0);

    EXPECT_TRUE(LoadFile("advanced-window.json"));

    // EXPECT_TRUE(myWindow->x == 5);
}

TEST(AdvancedArray, MultiWindow)
{
    SettingManager::clear();
    Setting<std::vector<MyWindow>> myWindows("/a");

    // EXPECT_TRUE(myWindows->size() == 0);

    EXPECT_TRUE(LoadFile("advanced-window-multi.json"));

    // EXPECT_TRUE(myWindows->size() == 3);

    std::deque<int> values{5, 10, 15};

    /*
    for (const auto &window : myWindows.getArray()) {
        EXPECT_TRUE(window.x == values.front());
        values.pop_front();
    }
    */

    // myWindows.push_back(MyWindow{});

    // EXPECT_TRUE(myWindows->size() == 4);

    // myWindows.push_back(MyWindow{});

    // EXPECT_TRUE(myWindows->size() == 5);
}

TEST(AdvancedArray, MultiFiles)
{
    auto sm1 = std::make_shared<SettingManager>();
    auto sm2 = std::make_shared<SettingManager>();
    Setting<std::vector<MyWindow>> myWindows("/a", SettingOption::Default, sm1);

    // EXPECT_TRUE(myWindows->size() == 0);

    EXPECT_TRUE(sm1->load("files/advanced-window-multi.json") ==
                SettingManager::LoadError::NoError);
    EXPECT_TRUE(sm2->load("files/advanced-window-multi.json") ==
                SettingManager::LoadError::NoError);

    // EXPECT_TRUE(myWindows->size() == 3);

    std::deque<int> values{5, 10, 15};

    /*
    for (const auto &window : myWindows.getArray()) {
        EXPECT_TRUE(window.x == values.front());
        values.pop_front();
    }
    */

    // myWindows.push_back(MyWindow{});

    // EXPECT_TRUE(myWindows->size() == 4);

    // myWindows.push_back(MyWindow{});

    // EXPECT_TRUE(myWindows->size() == 5);

    Setting<std::vector<MyWindow>> myWindows2("/a", SettingOption::Default,
                                              sm2);

    MyWindow test;
    test.x = 77;

    // myWindows2.push_back(std::move(test));

    sm1->setPath("lol.json");

    sm2->setPath("lol2.json");
}

namespace pajlada {

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

        ret.AddMember(rapidjson::Value("x", a).Move(),
                      pajlada::Serialize<int>::get(value.x, a), a);

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
            return ret;
        }

        if (value.HasMember("x")) {
            bool innerError = false;
            auto out = pajlada::Deserialize<int>::get(value["x"], &innerError);

            if (!innerError) {
                ret.x = out;
            }
        }

        return ret;
    }
};

}  // namespace pajlada
