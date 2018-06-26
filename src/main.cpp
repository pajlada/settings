#include "test/channel.hpp"
#include "test/channelmanager.hpp"
#include "test/foo.hpp"
#include "testhelpers.hpp"

#include "pajlada/settings/serialize.hpp"
#include "pajlada/settings/setting.hpp"
#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"

#define CATCH_CONFIG_MAIN
#include "libs/catch.hpp"

#include <cassert>
#include <iostream>
#include <string>

using namespace pajlada::Settings;
using namespace pajlada::test;

TEST_CASE("AdvancedSignals", "[settings]")
{
    int count = 0;
    auto cb = [&count](auto, auto) { ++count; };

    {
        count = 0;
        Setting<int> a("/advancedSignals/a");
        REQUIRE(count == 0);

        // c1
        a.connect(cb, false);

        REQUIRE(count == 0);

        // c2
        a.connect(cb, true);

        REQUIRE(count == 1);

        auto c3 = a.getValueChangedSignal().connect(cb);

        REQUIRE(count == 1);

        // c1, c2, and c3 are active
        a = 1;

        REQUIRE(count == 4);

        // c1, c2, and c3 are active
        // Value wasn't changed, hence no signal fired
        a = 1;

        REQUIRE(count == 4);

        // c1, c2, and c3 are active
        a = 2;

        REQUIRE(count == 7);

        REQUIRE(c3.isConnected());
        REQUIRE(c3.disconnect());
        REQUIRE(!c3.isConnected());
        REQUIRE(!c3.disconnect());
        REQUIRE(!c3.isConnected());

        // c1 and c2 c3 are active
        a = 3;

        REQUIRE(count == 9);
    }

    {
        count = 0;
        Setting<int> b("/advancedSignals/b");
        REQUIRE(count == 0);

        auto c1 = b.getValueChangedSignal().connect(cb);

        REQUIRE(count == 0);

        b = 1;

        REQUIRE(count == 1);

        REQUIRE(!c1.isBlocked());
        REQUIRE(c1.block());
        REQUIRE(c1.isBlocked());
        REQUIRE(!c1.block());
        REQUIRE(c1.isBlocked());

        b = 2;

        REQUIRE(count == 1);

        REQUIRE(c1.isBlocked());
        REQUIRE(c1.unblock());
        REQUIRE(!c1.isBlocked());
        REQUIRE(!c1.unblock());
        REQUIRE(!c1.isBlocked());

        b = 3;

        REQUIRE(count == 2);

        {
            auto c2 = b.getValueChangedSignal().connect(cb);

            REQUIRE(count == 2);

            b = 4;

            REQUIRE(count == 4);

            REQUIRE(c2.block());
            REQUIRE(c2.isBlocked());
            REQUIRE(!c1.isBlocked());

            b = 5;

            REQUIRE(count == 5);

            REQUIRE(c2.unblock());
            REQUIRE(!c2.isBlocked());
            REQUIRE(!c1.isBlocked());

            b = 6;

            REQUIRE(count == 7);

            REQUIRE(c1.isConnected());
            REQUIRE(c2.isConnected());
            REQUIRE(c2.disconnect());
            REQUIRE(c1.isConnected());
            REQUIRE(!c2.isConnected());

            b = 7;

            REQUIRE(count == 8);

            {
                auto c3 = b.getValueChangedSignal().connect(cb);

                REQUIRE(count == 8);

                b = 8;

                REQUIRE(count == 10);
            }

            // Connection c3 has gone out of scope, this has caused
            // a leaky callback. For now I think this should be
            // your own responsibility. This can easily be fixed
            // by just making a ScopedConnection instead like this:
            // Signals::ScopedConnection c3 = b.getValueChangedSignal().connect(cb);
            // Which will disconnect automatically when it goes out of scope

            // c1 and c3 are active
            b = 9;

            REQUIRE(count == 12);

            {
                pajlada::Signals::ScopedConnection c4 =
                    b.getValueChangedSignal().connect(cb);

                // c1, c3, and c4 are active
                b = 10;

                REQUIRE(count == 15);

                // c4 disconnected
            }

            // c1 and c3 are active
            b = 11;

            REQUIRE(count == 17);
        }
    }

    {
        count = 0;

        std::vector<pajlada::Signals::ScopedConnection> connections;

        {
            Setting<int> c("/advancedSignals/c");

            REQUIRE(count == 0);

            c.connect(cb, connections, false);
            c.connect(cb, connections, false);
            c.connect(cb, connections, false);

            REQUIRE(count == 0);

            // c1, c2, and c3 are active
            c = 1;

            REQUIRE(count == 3);

            connections.pop_back();

            // c1 and c2 are active
            c = 2;

            REQUIRE(count == 5);
        }

        // Scenario: Setting has gone out of scope, but we still have connections that point
        // toward a body that lived in the settings value changed signal

        // not able to assert this correctness, but if it's broken it SHOULD crash here
    }

    {
        count = 0;

        std::vector<pajlada::Signals::ScopedConnection> connections;
        Setting<int> d("/advancedSignals/d");
        REQUIRE(count == 0);

        d.connect(cb, connections, false);
        d.connect(cb, connections, false);
        d.connect(cb, connections, false);

        REQUIRE(count == 0);

        // c1, c2, and c3 are active
        d = 1;

        REQUIRE(count == 3);

        connections.pop_back();

        // c1 and c2 are active
        d = 2;

        REQUIRE(count == 5);

        connections.clear();

        // No connection is active
        d = 3;

        REQUIRE(count == 5);
    }

    {
        count = 0;

        std::vector<pajlada::Signals::ScopedConnection> connections;
        Setting<int> e("/advancedSignals/e");
        REQUIRE(count == 0);

        e.connect(cb, connections, false);
        e.connect(cb, connections, false);
        e.connect(cb, connections, false);

        REQUIRE(count == 0);

        // c1, c2, and c3 are active
        e = 1;

        REQUIRE(count == 3);

        connections.pop_back();

        // c1 and c2 are active
        e = 2;

        REQUIRE(count == 5);

        connections.clear();

        // No connection is active

        // TODO: If the setting is removed, no need to assert here imo
        // so remove operator= assert
        e = 3;

        REQUIRE(count == 5);

        e.remove();
    }
}

TEST_CASE("ResetToDefault", "[settings]")
{
    // No custom default value, not available in the settings file
    Setting<int> noDefault("/noDefault");

    // Custom default value, not available in the settings file
    Setting<int> customDefault("/customDefault", 5);

    // No custom default value, saved in settings file as the same value
    Setting<int> loadedSameNoDefault("/loadedSameNoDefault");

    // No custom default value, saved in settings file as a different value
    Setting<int> loadedDifferentNoDefault("/loadedDifferentNoDefault");

    // Custom default value, saved in settings file as the same value
    Setting<int> loadedSameCustomDefault("/loadedSameCustomDefault", 5);

    // Custom default value, saved in settings file as a different value
    Setting<int> loadedDifferentCustomDefault("/loadedDifferentCustomDefault",
                                              5);

    REQUIRE(noDefault.getDefaultValue() == 0);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    // Before loading
    REQUIRE(noDefault == 0);
    REQUIRE(customDefault == 5);
    REQUIRE(loadedSameNoDefault == 0);
    REQUIRE(loadedDifferentNoDefault == 0);
    REQUIRE(loadedSameCustomDefault == 5);
    REQUIRE(loadedDifferentCustomDefault == 5);

    // Before loading, after resetting all values to their default value
    noDefault.resetToDefaultValue();
    customDefault.resetToDefaultValue();
    loadedSameNoDefault.resetToDefaultValue();
    loadedDifferentNoDefault.resetToDefaultValue();
    loadedSameCustomDefault.resetToDefaultValue();
    loadedDifferentCustomDefault.resetToDefaultValue();

    REQUIRE(noDefault.getDefaultValue() == 0);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    REQUIRE(noDefault == 0);
    REQUIRE(customDefault == 5);
    REQUIRE(loadedSameNoDefault == 0);
    REQUIRE(loadedDifferentNoDefault == 0);
    REQUIRE(loadedSameCustomDefault == 5);
    REQUIRE(loadedDifferentCustomDefault == 5);

    REQUIRE(LoadFile("in.resettodefault.json"));

    // After loading
    REQUIRE(noDefault == 0);
    REQUIRE(customDefault == 5);
    REQUIRE(loadedSameNoDefault == 0);
    REQUIRE(loadedDifferentNoDefault == 1);
    REQUIRE(loadedSameCustomDefault == 5);
    REQUIRE(loadedDifferentCustomDefault == 6);

    REQUIRE(noDefault.getDefaultValue() == 0);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    // Reset all values to their default values
    noDefault.resetToDefaultValue();
    customDefault.resetToDefaultValue();
    loadedSameNoDefault.resetToDefaultValue();
    loadedDifferentNoDefault.resetToDefaultValue();
    loadedSameCustomDefault.resetToDefaultValue();
    loadedDifferentCustomDefault.resetToDefaultValue();

    REQUIRE(noDefault.getDefaultValue() == 0);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    // After loading, after resetting all values to their default value
    REQUIRE(noDefault == 0);
    REQUIRE(customDefault == 5);
    REQUIRE(loadedSameNoDefault == 0);
    REQUIRE(loadedDifferentNoDefault == 0);
    REQUIRE(loadedSameCustomDefault == 5);
    REQUIRE(loadedDifferentCustomDefault == 5);

    noDefault.setDefaultValue(1);

    REQUIRE(noDefault == 0);

    noDefault.resetToDefaultValue();

    REQUIRE(noDefault == 1);

    REQUIRE(noDefault.getDefaultValue() == 1);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);
}

TEST_CASE("Any", "[settings]")
{
    Setting<boost::any> test("/anyTest");
    auto test2 = new Setting<boost::any>("/anyTest2");

    auto v1 = test.getValue();
    auto v2 = test2->getValue();
}

TEST_CASE("IsEqual", "[settings]")
{
    Setting<std::map<std::string, std::string>> stringMap("/stringMap");
    Setting<std::map<std::string, boost::any>> anyMap("/anyMap");

    auto v = anyMap.getValue();

    REQUIRE(LoadFile("in.isequal.json"));

    auto v2 = anyMap.getValue();

    {
        auto map = stringMap.getValue();
        REQUIRE(map.size() == 3);
        REQUIRE(map["a"] == "5");
        REQUIRE(map["b"] == "10");
        REQUIRE(map["c"] == "15");

        int numSignalsFired = 0;

        stringMap.getValueChangedSignal().connect(
            [&numSignalsFired](auto, auto) {
                ++numSignalsFired;  //
            });

        REQUIRE(numSignalsFired == 0);

        stringMap = map;

        REQUIRE(numSignalsFired == 0);

        map["a"] = "8";

        stringMap = map;

        REQUIRE(numSignalsFired == 1);
    }

    {
        using boost::any_cast;

        auto map = anyMap.getValue();
        REQUIRE(map.size() == 3);
        REQUIRE(any_cast<std::string>(map["a"]) == "5");
        REQUIRE(any_cast<int>(map["b"]) == 10);
        REQUIRE(any_cast<std::string>(map["c"]) == "15");

        int numSignalsFired = 0;

        anyMap.getValueChangedSignal().connect([&numSignalsFired](auto, auto) {
            ++numSignalsFired;  //
        });

        REQUIRE(numSignalsFired == 0);

        anyMap = map;

        // Because we use boost::any, we cannot know whether the map that we're
        // setting it to has been changed or not, so the equality check will
        // always return false to be safe
        REQUIRE(numSignalsFired == 1);

        map["a"] = "8";

        anyMap = map;

        REQUIRE(numSignalsFired == 2);
    }
}

TEST_CASE("Simple Map", "[settings]")
{
    using boost::any_cast;

    Setting<std::map<std::string, boost::any>> test("/map");

    REQUIRE(LoadFile("in.simplemap.json"));

    auto map = test.getValue();
    REQUIRE(map.size() == 3);
    REQUIRE(any_cast<int>(map["a"]) == 1);
    REQUIRE(any_cast<std::string>(map["b"]) == "asd");
    REQUIRE(any_cast<double>(map["c"]) == 3.14);

    REQUIRE(SettingManager::saveAs("files/out.simplemap.json"));
}

TEST_CASE("Complex Map", "[settings]")
{
    using boost::any_cast;

    Setting<std::map<std::string, boost::any>> test("/map");

    REQUIRE(LoadFile("in.complexmap.json"));

    auto map = test.getValue();
    REQUIRE(map.size() == 3);
    REQUIRE(any_cast<int>(map["a"]) == 5);

    auto innerMap =
        any_cast<std::map<std::string, boost::any>>(map["innerMap"]);
    REQUIRE(innerMap.size() == 3);
    REQUIRE(any_cast<int>(innerMap["a"]) == 420);
    REQUIRE(any_cast<int>(innerMap["b"]) == 320);
    REQUIRE(any_cast<double>(innerMap["c"]) == 13.37);

    auto innerArray = any_cast<std::vector<boost::any>>(map["innerArray"]);
    REQUIRE(innerArray.size() == 9);
    REQUIRE(any_cast<int>(innerArray[0]) == 1);
    REQUIRE(any_cast<int>(innerArray[1]) == 2);
    REQUIRE(any_cast<int>(innerArray[2]) == 3);
    REQUIRE(any_cast<int>(innerArray[3]) == 4);
    REQUIRE(any_cast<std::string>(innerArray[4]) == "testman");
    REQUIRE(any_cast<bool>(innerArray[5]) == true);
    REQUIRE(any_cast<bool>(innerArray[6]) == false);
    REQUIRE(any_cast<double>(innerArray[7]) == 4.20);

    auto innerArrayMap =
        any_cast<std::map<std::string, boost::any>>(innerArray[8]);
    REQUIRE(innerArrayMap.size() == 3);
    REQUIRE(any_cast<int>(innerArrayMap["a"]) == 1);
    REQUIRE(any_cast<int>(innerArrayMap["b"]) == 2);
    REQUIRE(any_cast<int>(innerArrayMap["c"]) == 3);

    REQUIRE(SettingManager::saveAs("files/out.complexmap.json"));
}

TEST_CASE("Array test", "[settings]")
{
    Setting<int> test1("/array/0/int");
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
    REQUIRE(LoadFile("in.array_size.json"));

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

    REQUIRE(LoadFile("in.vector.json"));

    auto vec = test.getValue();

    REQUIRE(vec.size() == 3);

    REQUIRE(vec.at(0) == 5);
    REQUIRE(vec.at(1) == 10);
    REQUIRE(vec.at(2) == 15);

    std::vector<int> x = {1, 2, 3};

    test = x;

    REQUIRE(SettingManager::saveAs("files/out.vector.json") == true);
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
        [&maxMessageLength](const int &newValue, auto) {
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

    REQUIRE(LoadFile("channelmanager.json"));

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
    REQUIRE(LoadFile("d.channels.json"));

    // Post defaults load
    REQUIRE(chHemirt.maxMessageLength.getValue() == 200);
    REQUIRE(chPajlada.maxMessageLength == 240);

    // Load custom file
    REQUIRE(LoadFile("channels.json"));

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
    REQUIRE(Foo::f7.getValue() == 1.f);

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

    REQUIRE(LoadFile("test.json"));

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

    Foo::f7 = 0.f;

    REQUIRE(SettingManager::saveAs("files/out.test2.json") == true);
}

TEST_CASE("Deserialize", "[settings]")
{
    SettingManager::clear();
    REQUIRE(LoadFile("deserialize-samples.json"));

    SECTION("int")
    {
        Setting<int> *a3, *a4;
        Setting<int> a1("/int");
        Setting<int> a2("/float");
        DD_THROWS(a3 = new Setting<int>("/string"));
        DD_THROWS(a4 = new Setting<int>("/bool"));

        REQUIRE(a1.getValue() == 5);
        REQUIRE(a2.getValue() == 5);
        REQUIRE_IF_NOEXCEPT(a3, 0);
        REQUIRE_IF_NOEXCEPT(a4, 0);
    }

    SECTION("float")
    {
        Setting<float> *a3, *a4;
        Setting<float> a1("/int");
        Setting<float> a2("/float");
        DD_THROWS(a3 = new Setting<float>("/string"));
        DD_THROWS(a4 = new Setting<float>("/bool"));

        REQUIRE(a1.getValue() == Approx(5.f));
        REQUIRE(a2.getValue() == Approx(5.3f));
        REQUIRE_IF_NOEXCEPT(a3, 0);
        REQUIRE_IF_NOEXCEPT(a4, 0);
    }

    SECTION("double")
    {
        Setting<double> *a3 = nullptr;
        Setting<double> *a4 = nullptr;
        Setting<double> a1("/int");
        Setting<double> a2("/float");
        DD_THROWS(a3 = new Setting<double>("/string"));
        DD_THROWS(a4 = new Setting<double>("/bool"));

        REQUIRE(a1.getValue() == Approx(5.));
        REQUIRE(a2.getValue() == Approx(5.3));
        REQUIRE_IF_NOEXCEPT(a3, 0);
        REQUIRE_IF_NOEXCEPT(a4, 0);
    }

    SECTION("bool")
    {
        Setting<bool> *a2, *a3, *a4;
        Setting<bool> a1("/int");
        DD_THROWS(a2 = new Setting<bool>("/float"));
        DD_THROWS(a3 = new Setting<bool>("/string"));
        a4 = new Setting<bool>("/bool");

        Setting<bool> a5("/int1");

        REQUIRE(a1.getValue() == false);
        REQUIRE_IF_NOEXCEPT(a2, false);
        REQUIRE_IF_NOEXCEPT(a3, false);
        REQUIRE(a4->getValue() == true);
        REQUIRE(a5.getValue() == true);
    }

    SECTION("string")
    {
        Setting<std::string> *a1, *a2, *a3, *a4;
        DD_THROWS(a1 = new Setting<std::string>("/int"));
        DD_THROWS(a2 = new Setting<std::string>("/float"));
        a3 = new Setting<std::string>("/string");
        DD_THROWS(a4 = new Setting<std::string>("/bool"));

        REQUIRE_IF_NOEXCEPT(a1, "");
        REQUIRE_IF_NOEXCEPT(a2, "");
        REQUIRE(a3->getValue() == "xd");
        REQUIRE_IF_NOEXCEPT(a4, "");
    }
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
