#include "test/channel.hpp"
#include "test/channelmanager.hpp"
#include "test/foo.hpp"

#include "pajlada/settings/serialize.hpp"
#include "pajlada/settings/setting.hpp"
#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"

#define CATCH_CONFIG_MAIN
#include "libs/catch.hpp"

#include <assert.h>
#include <iostream>

using namespace pajlada;
using namespace pajlada::Settings;
using namespace pajlada::test;

class SimpleCustomClass
{
public:
    SimpleCustomClass() = default;

    explicit SimpleCustomClass(int _x, int _y)
        : x(_x)
        , y(_y)
    {
    }

    int x = 0;
    int y = 0;

    bool
    operator==(const SimpleCustomClass &o) const
    {
        return std::tie(this->x, this->y) == std::tie(o.x, o.y);
    }

    bool
    operator!=(const SimpleCustomClass &o) const
    {
        return !(*this == o);
    }
};

namespace pajlada {
namespace Settings {

template <>
struct Serialize<SimpleCustomClass> {
    static rapidjson::Value
    get(const SimpleCustomClass &value, rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        ret.AddMember(rapidjson::Value("x", a).Move(),
                      Serialize<int>::get(value.x, a), a);
        ret.AddMember(rapidjson::Value("y", a).Move(),
                      Serialize<int>::get(value.y, a), a);

        return ret;
    }
};

template <>
struct Deserialize<SimpleCustomClass> {
    static SimpleCustomClass
    get(const rapidjson::Value &value)
    {
        SimpleCustomClass ret;

        // Note: missing error checking here
        // value might not be an object, and value might not have the members
        // "x" and "y"
        ret.x = Deserialize<int>::get(value["x"]);
        ret.y = Deserialize<int>::get(value["y"]);

        return ret;
    }
};

}  // namespace Settings
}  // namespace pajlada

TEST_CASE("Array test", "[settings]")
{
    Setting<int> test1("/array/0/int", SettingOption::SaveOnChange);
    Setting<int> test2("/array/1/int", SettingOption::SaveInitialValue);
    Setting<int> test3("/array/2/int", SettingOption::SaveInitialValue);

    test1 = 5;
    test2 = 10;
    test3 = 15;

    // This below assertation is not always true
    // It will only be true if the settings above area created with
    // "SaveInitialValue", or if "SaveOnChange" is enabled and the value has
    // been changed
    REQUIRE(SettingManager::arraySize("/array") == 3);

    REQUIRE(SettingManager::saveAs("files/out.array_test.json") == true);

    REQUIRE(SettingManager::arraySize("/array") == 3);
}

TEST_CASE("Array size", "[settings]")
{
    REQUIRE(SettingManager::loadFrom("files/in.array_size.json") ==
            SettingManager::LoadError::NoError);

    REQUIRE(SettingManager::arraySize("/arraySize1") == 1);
    REQUIRE(SettingManager::arraySize("/arraySize2") == 2);
    REQUIRE(SettingManager::arraySize("/arraySize3") == 3);
    REQUIRE(SettingManager::arraySize("/arraySize4") == 4);

    // Not an array
    REQUIRE(SettingManager::arraySize("/arraySize5") == 0);
}

TEST_CASE("Vector", "[settings]")
{
    Setting<std::vector<int>> test("/vectorTest");

    REQUIRE(SettingManager::loadFrom("files/in.vector.json") ==
            SettingManager::LoadError::NoError);

    auto vec = test.getValue();

    REQUIRE(vec.size() == 3);

    REQUIRE(vec.at(0) == 5);
    REQUIRE(vec.at(1) == 10);
    REQUIRE(vec.at(2) == 15);

    std::vector<int> x = {1, 2, 3};

    test = x;

    REQUIRE(SettingManager::saveAs("files/out.vector.json") == true);
}

TEST_CASE("Borrowed setting", "[settings]")
{
    Setting<int> test("/borrowedSettingInt");

    int numSignalsFired = 0;

    test.getValueChangedSignal().connect(
        [&numSignalsFired](const auto &newValue) {
            ++numSignalsFired;  //
        });

    test = 10;

    REQUIRE(test == 10);
    REQUIRE(numSignalsFired == 1);

    {
        BorrowedSetting<int> borrowedTest = test.borrow();

        borrowedTest = 15;
    }

    REQUIRE(test == 15);
    REQUIRE(numSignalsFired == 2);
}

TEST_CASE("Borrowed setting custom class", "[settings]")
{
    Setting<SimpleCustomClass> test("/borrowedSettingCustom");

    int numSignalsFired = 0;

    test.getValueChangedSignal().connect(
        [&numSignalsFired](const auto &newValue) {
            ++numSignalsFired;  //
        });

    REQUIRE(numSignalsFired == 0);

    test = SimpleCustomClass{5, 10};

    REQUIRE(numSignalsFired == 1);

    {
        auto v = test.getValue();
        REQUIRE(v.x == 5);
        REQUIRE(v.y == 10);
    }

    {
        BorrowedSetting<SimpleCustomClass> borrowedTest = test.borrow();
    }

    REQUIRE(numSignalsFired == 1);

    {
        auto v = test.getValue();
        REQUIRE(v.x == 5);
        REQUIRE(v.y == 10);
    }

    {
        BorrowedSetting<SimpleCustomClass> borrowedTest = test.borrow();

        borrowedTest->x = 6;
    }

    REQUIRE(numSignalsFired == 2);

    {
        auto v = test.getValue();
        REQUIRE(v.x == 6);
        REQUIRE(v.y == 10);
    }
}

TEST_CASE("Scoped settings", "[settings]")
{
    Setting<int> a1("/a", 1);

    REQUIRE(a1 == 1);

    {
        Setting<int> a2("/a");
        // Because /a is already initialized, we should just load the same
        // shared_ptr that a1 uses

        REQUIRE(a2 == 1);

        a2 = 8;

        REQUIRE(a2 == 8);
        REQUIRE(a1 == 8);
    }

    REQUIRE(a1 == 8);

    Setting<int>::set("/a", 10);

    REQUIRE(a1 == 10);

    {
        Setting<int> a2("/a");

        REQUIRE(a2 == 10);

        Setting<int>::set("/a", 20);

        REQUIRE(a2 == 20);
        REQUIRE(a1 == 20);
    }

    REQUIRE(a1 == 20);
}

// TODO: Re-implement scoped tests

TEST_CASE("Signals", "[settings]")
{
    Channel ch("xD");

    int maxMessageLength = 0;

    ch.maxMessageLength.getValueChangedSignal().connect(
        [&maxMessageLength](const int &newValue) {
            maxMessageLength = newValue;  //
        });

    ch.maxMessageLength = 1;
    REQUIRE(maxMessageLength == 1);

    ch.maxMessageLength = 2;
    REQUIRE(maxMessageLength == 2);

    ch.maxMessageLength = 3;
    REQUIRE(maxMessageLength == 3);

    ch.maxMessageLength = 4;
    REQUIRE(maxMessageLength == 4);
}

TEST_CASE("ChannelManager", "[settings]")
{
    ChannelManager manager;

    REQUIRE(manager.channels.size() == pajlada::test::NUM_CHANNELS);

    for (size_t i = 0; i < manager.channels.size(); ++i) {
        REQUIRE(manager.channels.at(i).name.getValue() == "Name not loaded");
    }

    REQUIRE(SettingManager::loadFrom("files/channelmanager.json") ==
            SettingManager::LoadError::NoError);

    REQUIRE(manager.channels.at(0).name.getValue() == "pajlada");
    REQUIRE(manager.channels.at(1).name.getValue() == "hemirt");
    REQUIRE(manager.channels.at(2).name.getValue() == "gempir");

    // Last channel should always be unset
    REQUIRE(
        manager.channels.at(pajlada::test::NUM_CHANNELS - 1).name.getValue() ==
        "Name not loaded");

    for (auto i = 4; i < pajlada::test::NUM_CHANNELS; ++i) {
        manager.channels.at(i).name = "From file FeelsGoodMan";
    }

    REQUIRE(manager.channels.size() == pajlada::test::NUM_CHANNELS);
    REQUIRE(SettingManager::saveAs("files/out.test3.json"));
    REQUIRE(manager.channels.size() == pajlada::test::NUM_CHANNELS);
}

TEST_CASE("Channel", "[settings]")
{
    Channel chHemirt("hemirt");
    Channel chPajlada("pajlada");

    // Pre-load
    REQUIRE(chHemirt.maxMessageLength == 240);
    REQUIRE(chPajlada.maxMessageLength == 240);

    // Load default file
    REQUIRE(SettingManager::loadFrom("files/d.channels.json") ==
            SettingManager::LoadError::NoError);

    // Post defaults load
    REQUIRE(chHemirt.maxMessageLength.getValue() == 200);
    REQUIRE(chPajlada.maxMessageLength == 240);

    // Load custom file
    REQUIRE(SettingManager::loadFrom("files/channels.json") ==
            SettingManager::LoadError::NoError);

    // Post channels load
    REQUIRE(chHemirt.maxMessageLength == 300);
    REQUIRE(chPajlada.maxMessageLength == 500);
}

TEST_CASE("Load files", "[settings]")
{
    SECTION("Invalid files")
    {
        REQUIRE(SettingManager::loadFrom("files/bad-1.json") ==
                SettingManager::LoadError::JSONParseError);
        REQUIRE(SettingManager::loadFrom("files/bad-2.json") ==
                SettingManager::LoadError::JSONParseError);
        REQUIRE(SettingManager::loadFrom("files/bad-3.json") ==
                SettingManager::LoadError::JSONParseError);
        REQUIRE(SettingManager::loadFrom("files/empty.json") ==
                SettingManager::LoadError::NoError);
    }

    SECTION("Non-existant files")
    {
        REQUIRE(SettingManager::loadFrom("files/test-non-existant-file.json") ==
                SettingManager::LoadError::CannotOpenFile);
    }

    SECTION("Valid files")
    {
        REQUIRE(SettingManager::loadFrom("files/default.json") ==
                SettingManager::LoadError::NoError);
    }
}

TEST_CASE("Simple static", "[settings]")
{
    REQUIRE(Foo::i1.getValue() == 1);
    REQUIRE(Foo::i2.getValue() == 2);
    REQUIRE(Foo::i3.getValue() == 3);
    REQUIRE(Foo::s1.getValue() == "Default string");

    REQUIRE(Foo::f1.getValue() == 1.0101f);
    REQUIRE(Foo::f2.getValue() == 1.0101010101f);
    REQUIRE(Foo::f3.getValue() == 1.123456789f);
    REQUIRE(Foo::f4.getValue() == 1.f);
    REQUIRE(Foo::f5.getValue() == 0.f);
    REQUIRE(Foo::f6.getValue() == -.1f);

    REQUIRE(Foo::d1.getValue() == 1.0101);
    REQUIRE(Foo::d2.getValue() == 1.0101010101);
    REQUIRE(Foo::d3.getValue() == 1.123456789);
    REQUIRE(Foo::d4.getValue() == 1.);
    REQUIRE(Foo::d5.getValue() == 0.);
    REQUIRE(Foo::d6.getValue() == -.1);
    REQUIRE(Foo::d7.getValue() == 123.456);

    REQUIRE(Foo::b1.getValue() == true);
    REQUIRE(Foo::b2.getValue() == false);
    REQUIRE(Foo::b3.getValue() == true);
    REQUIRE(Foo::b4.getValue() == false);
    REQUIRE(Foo::b5.getValue() == true);
    REQUIRE(Foo::b6.getValue() == false);
    REQUIRE(Foo::b7.getValue() == true);

    REQUIRE(Foo::rootInt1.getValue() == 1);
    REQUIRE(Foo::rootInt2.getValue() == 1);

    REQUIRE(SettingManager::loadFrom("files/test.json") ==
            SettingManager::LoadError::NoError);

    // Floats post-load
    REQUIRE(Foo::f1.getValue() == 1.f);
    REQUIRE(Foo::f2.getValue() == -1.f);
    REQUIRE(Foo::f3.getValue() == 500.987654321f);
    REQUIRE(Foo::f4.getValue() == 5.1f);
    REQUIRE(Foo::f5.getValue() == 5.321f);
    REQUIRE(Foo::f6.getValue() == 500000.1f);

    // Doubles post-load
    REQUIRE(Foo::d1.getValue() == 1.);
    REQUIRE(Foo::d2.getValue() == -1.);
    REQUIRE(Foo::d3.getValue() == 500.987654321);
    REQUIRE(Foo::d4.getValue() == 5.1);
    REQUIRE(Foo::d5.getValue() == 5.321);
    REQUIRE(Foo::d6.getValue() == 500000.1);
    REQUIRE(Foo::d7.getValue() == 123.456);

    // Booleans post-load
    REQUIRE(Foo::b1.getValue() == false);  // false
    REQUIRE(Foo::b2.getValue() == true);   // true
    REQUIRE(Foo::b3.getValue() == false);  // 0
    REQUIRE(Foo::b4.getValue() == true);   // 1
    REQUIRE(Foo::b5.getValue() == false);  // 50291 (anything but 1)
    REQUIRE(Foo::b6.getValue() == false);  // nothing loaded
    REQUIRE(Foo::b7.getValue() == true);   // nothing loaded

    REQUIRE(Foo::i1.getValue() == 2);
    REQUIRE(Foo::i2.getValue() == 3);
    REQUIRE(Foo::i3.getValue() == 4);
    REQUIRE(Foo::s1.getValue() == "Loaded string");
    REQUIRE(Foo::rootInt1.getValue() == 2);
    REQUIRE(Foo::rootInt2.getValue() == 2);

    // Class2::i1.signalValueChanged.connect([](const int &newValue) {
    //     std::cout << "new value for i1: " << newValue << std::endl;
    // });

    Foo::rootInt1 = 3;

    Foo::i1.setValue(3);

    REQUIRE(Foo::i1.getValue() == 3);

    Foo::i1 = 4;

    REQUIRE(Foo::i1.getValue() == 4);

    REQUIRE(SettingManager::saveAs("files/out.test2.json") == true);
}

// NOTE: This must be at the bottom to not disrupt with other
// static-initialization testing above
TEST_CASE("Test save/loading stuff", "[settings]")
{
    Setting<int>::set("/asd", 5);

    SettingManager::clear();

    Setting<int>::set("/lol", 10, SettingOption::DoNotWriteToJSON);

    REQUIRE(SettingManager::saveAs("files/out.test.json") == true);
}
