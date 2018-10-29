// UNSORTED TESTS

#include "test/channel.hpp"
#include "test/channelmanager.hpp"
#include "test/common.hpp"
#include "test/foo.hpp"

#include <pajlada/settings/serialize.hpp>
#include <pajlada/settings/setting.hpp>
#include <pajlada/settings/settingdata.hpp>
#include <pajlada/settings/settingmanager.hpp>

#include <cassert>
#include <iostream>
#include <string>

using namespace pajlada::Settings;
using namespace pajlada::test;
using namespace std;

TEST_CASE("Any", "[settings]")
{
    Setting<boost::any> test("/anyTest");
    auto test2 = new Setting<boost::any>("/anyTest2");

    auto v1 = test.getValue();
    auto v2 = test2->getValue();
}

TEST_CASE("Array test", "[settings]")
{
    Setting<int> test1("/array/0/int");
    Setting<int> test2("/array/1/int");
    Setting<int> test3("/array/2/int");

    test1 = 5;
    test2 = 10;
    test3 = 15;

    // This below assertation is not always true
    // It will only be true if the settings above area created with
    // "SaveInitialValue", or if "SaveOnChange" is enabled and the value has
    // been changed
    REQUIRE(SettingManager::arraySize("/array") == 3);

    REQUIRE(SettingManager::gSaveAs("files/out.array_test.json") == true);

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
    Setting<vector<int>> test("/vectorTest");

    REQUIRE(LoadFile("in.vector.json"));

    auto vec = test.getValue();

    REQUIRE(vec.size() == 3);

    REQUIRE(vec.at(0) == 5);
    REQUIRE(vec.at(1) == 10);
    REQUIRE(vec.at(2) == 15);

    vector<int> x = {1, 2, 3};

    test = x;

    REQUIRE(SettingManager::gSaveAs("files/out.vector.json") == true);
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
    REQUIRE(SettingManager::gSaveAs("files/out.test3.json"));
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
        REQUIRE(SettingManager::gLoadFrom("files/bad-1.json") ==
                SettingManager::LoadError::JSONParseError);
        REQUIRE(SettingManager::gLoadFrom("files/bad-2.json") ==
                SettingManager::LoadError::JSONParseError);
        REQUIRE(SettingManager::gLoadFrom("files/bad-3.json") ==
                SettingManager::LoadError::JSONParseError);
        REQUIRE(SettingManager::gLoadFrom("files/empty.json") ==
                SettingManager::LoadError::NoError);
    }

    SECTION("Non-existant files")
    {
        REQUIRE(
            SettingManager::gLoadFrom("files/test-non-existant-file.json") ==
            SettingManager::LoadError::CannotOpenFile);
    }

    SECTION("Valid files")
    {
        REQUIRE(SettingManager::gLoadFrom("files/default.json") ==
                SettingManager::LoadError::NoError);
    }
}

TEST_CASE("Misc", "[settings]")
{
    Setting<int> test1("/test");
    REQUIRE(test1.getPath() == "/test");
    REQUIRE(test1.getData().lock()->getPath() == "/test");
    REQUIRE(test1.getData().lock()->getPath() == test1.getPath());
}

// TODO(pajlada): Test disconnecting signals properly. See "createCheckBox in chatterino"
