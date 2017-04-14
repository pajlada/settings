#include "test/channel.hpp"
#include "test/channelmanager.hpp"
#include "test/foo.hpp"

#include "pajlada/settings/setting.hpp"
#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"

#define CATCH_CONFIG_MAIN
#include "libs/catch.hpp"

#include <assert.h>
#include <iostream>

using namespace pajlada;
using namespace pajlada::settings;
using namespace pajlada::test;

TEST_CASE("Signals", "[settings]")
{
    Channel ch("xD");

    int maxMessageLength = 0;

    ch.maxMessageLength.valueChanged.connect(
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

    for (auto i = 0; i < manager.channels.size(); ++i) {
        REQUIRE(manager.channels.at(i).name.getValue() == "Name not loaded");
    }

    REQUIRE(SettingManager::loadFrom("files/channelmanager.json"));

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
    REQUIRE(SettingManager::saveAs("files/test3.json"));
    REQUIRE(manager.channels.size() == pajlada::test::NUM_CHANNELS);
}

#ifndef ONLY_MINI_TEST
TEST_CASE("Channel", "[settings]")
{
    SettingManager::clear();

    Channel chHemirt("hemirt");
    Channel chPajlada("pajlada");

    // Pre-load
    REQUIRE(chHemirt.maxMessageLength == 240);
    REQUIRE(chPajlada.maxMessageLength == 240);

    // Load default file
    REQUIRE(SettingManager::loadFrom("files/d.channels.json") == true);

    // Post defaults load
    REQUIRE(chHemirt.maxMessageLength.getValue() == 200);
    REQUIRE(chPajlada.maxMessageLength == 240);

    // Load custom file
    REQUIRE(SettingManager::loadFrom("files/channels.json") == true);

    // Post channels load
    REQUIRE(chHemirt.maxMessageLength == 300);
    REQUIRE(chPajlada.maxMessageLength == 500);
}

TEST_CASE("Load files", "[settings]")
{
    SECTION("Invalid files")
    {
        REQUIRE(SettingManager::loadFrom("files/bad-1.json") == false);
        REQUIRE(SettingManager::loadFrom("files/bad-2.json") == false);
        REQUIRE(SettingManager::loadFrom("files/bad-3.json") == false);
        REQUIRE(SettingManager::loadFrom("files/empty.json") == false);
    }

    SECTION("Non-existant files")
    {
        REQUIRE(SettingManager::loadFrom("files/test-non-existant-file.json") ==
                false);
    }

    SECTION("Valid files")
    {
        REQUIRE(SettingManager::loadFrom("files/default.json") == true);
    }
}

TEST_CASE("Simple static", "[settings]")
{
    SettingManager::clear();

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

    REQUIRE(SettingManager::loadFrom("files/test.json") == true);

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

    REQUIRE(SettingManager::saveAs("files/test2.json") == true);
}
#endif
