#include <gtest/gtest.h>

#include <cassert>
#include <iostream>
#include <memory>
#include <pajlada/serialize.hpp>
#include <pajlada/settings/setting.hpp>
#include <pajlada/settings/settingdata.hpp>
#include <pajlada/settings/settingmanager.hpp>
#include <string>

#include "channel.hpp"
#include "channelmanager.hpp"
#include "common.hpp"
#include "foo.hpp"

using namespace pajlada::Settings;
using namespace pajlada::test;
using SaveResult = pajlada::Settings::SettingManager::SaveResult;

TEST(Misc, StdAny)
{
    Setting<std::any> test("/anyTest");
    std::unique_ptr<Setting<std::any>> test2(
        new Setting<std::any>("/anyTest2"));

    auto v1 = test.getValue();
    auto v2 = test2->getValue();
}

TEST(Misc, Array)
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
    EXPECT_TRUE(SettingManager::arraySize("/array") == 3);

    EXPECT_EQ(SaveResult::Success, SaveFile("out.array_test.json"));

    EXPECT_TRUE(SettingManager::arraySize("/array") == 3);
}

TEST(Misc, ArraySize)
{
    EXPECT_TRUE(LoadFile("in.array_size.json"));

    EXPECT_TRUE(SettingManager::arraySize("/arraySize1") == 1);
    EXPECT_TRUE(SettingManager::arraySize("/arraySize2") == 2);
    EXPECT_TRUE(SettingManager::arraySize("/arraySize3") == 3);
    EXPECT_TRUE(SettingManager::arraySize("/arraySize4") == 4);

    // Not an array
    EXPECT_TRUE(SettingManager::arraySize("/arraySize5") == 0);
}

TEST(Misc, Vector)
{
    Setting<std::vector<int>> test("/vectorTest");

    EXPECT_TRUE(LoadFile("in.vector.json"));

    auto vec = test.getValue();

    EXPECT_TRUE(vec.size() == 3);

    EXPECT_TRUE(vec.at(0) == 5);
    EXPECT_TRUE(vec.at(1) == 10);
    EXPECT_TRUE(vec.at(2) == 15);

    std::vector<int> x = {1, 2, 3};

    test = x;

    EXPECT_EQ(SaveResult::Success, SaveFile("out.vector.json"));
}

TEST(Misc, ChannelManager)
{
    ChannelManager manager;

    EXPECT_TRUE(manager.channels.size() == pajlada::test::NUM_CHANNELS);

    for (size_t i = 0; i < manager.channels.size(); ++i) {
        EXPECT_TRUE(manager.channels.at(i).name.getValue() ==
                    "Name not loaded");
    }

    EXPECT_TRUE(LoadFile("channelmanager.json"));

    EXPECT_TRUE(manager.channels.at(0).name.getValue() == "pajlada");
    EXPECT_TRUE(manager.channels.at(1).name.getValue() == "hemirt");
    EXPECT_TRUE(manager.channels.at(2).name.getValue() == "gempir");

    // Last channel should always be unset
    EXPECT_TRUE(
        manager.channels.at(pajlada::test::NUM_CHANNELS - 1).name.getValue() ==
        "Name not loaded");

    for (auto i = 4; i < pajlada::test::NUM_CHANNELS; ++i) {
        manager.channels.at(i).name = "From file FeelsGoodMan";
    }

    EXPECT_TRUE(manager.channels.size() == pajlada::test::NUM_CHANNELS);
    EXPECT_EQ(SaveResult::Success, SaveFile("out.test3.json"));
    EXPECT_TRUE(manager.channels.size() == pajlada::test::NUM_CHANNELS);
}

TEST(Misc, Channel)
{
    Channel chHemirt("hemirt");
    Channel chPajlada("pajlada");

    // Pre-load
    EXPECT_TRUE(chHemirt.maxMessageLength == 240);
    EXPECT_TRUE(chPajlada.maxMessageLength == 240);

    // Load default file
    EXPECT_TRUE(LoadFile("d.channels.json"));

    // Post defaults load
    EXPECT_TRUE(chHemirt.maxMessageLength.getValue() == 200);
    EXPECT_TRUE(chPajlada.maxMessageLength == 240);

    // Load custom file
    EXPECT_TRUE(LoadFile("channels.json"));

    // Post channels load
    EXPECT_TRUE(chHemirt.maxMessageLength == 300);
    EXPECT_TRUE(chPajlada.maxMessageLength == 500);
}

TEST(Misc, LoadFilesInvalidFiles)
{
    EXPECT_TRUE(SettingManager::gLoadFrom("files/bad-1.json") ==
                SettingManager::LoadError::JSONParseError);
    EXPECT_TRUE(SettingManager::gLoadFrom("files/bad-2.json") ==
                SettingManager::LoadError::JSONParseError);
    EXPECT_TRUE(SettingManager::gLoadFrom("files/bad-3.json") ==
                SettingManager::LoadError::JSONParseError);
    EXPECT_TRUE(SettingManager::gLoadFrom("files/empty.json") ==
                SettingManager::LoadError::NoError);
}

TEST(Misc, NonExistantFiles)
{
    EXPECT_TRUE(
        SettingManager::gLoadFrom("files/test-non-existant-file.json") ==
        SettingManager::LoadError::CannotOpenFile);
}

TEST(Misc, ValidFiles)
{
    EXPECT_TRUE(SettingManager::gLoadFrom("files/default.json") ==
                SettingManager::LoadError::NoError);
}

TEST(Misc, Misc)
{
    Setting<int> test1("/test");
    EXPECT_TRUE(test1.getPath() == "/test");
    EXPECT_TRUE(test1.getData().lock()->getPath() == "/test");
    EXPECT_TRUE(test1.getData().lock()->getPath() == test1.getPath());
}

TEST(Misc, Stringify)
{
    rapidjson::Value test(5);

    EXPECT_TRUE(SettingManager::stringify(test) == "5");
}

TEST(Misc, MoveSet)
{
    int v = 69;
    Setting<int> test1("/test");
    EXPECT_TRUE(test1 == 0);
    test1 = 3;
    EXPECT_TRUE(test1 == 3);
    test1 = std::move(v);
    EXPECT_TRUE(test1 == 69);

    std::string v2("lol");
    Setting<std::string> test2("/test");
    EXPECT_TRUE(test2 == "");
    EXPECT_TRUE(test2.getValue() == "");
    test2 = "hej";
    EXPECT_TRUE(test2 == "hej");
    EXPECT_TRUE(test2.getValue() == "hej");
    test2 = std::move(v2);
    EXPECT_TRUE(test2 == "lol");
    EXPECT_TRUE(test2.getValue() == "lol");

    const auto &lol = test2;

    const auto &hehe = lol.getValue();
    EXPECT_TRUE(hehe == "lol");
    EXPECT_TRUE(lol.getValue() == "lol");
    EXPECT_TRUE(lol.getValue() == "lol");
    EXPECT_TRUE(lol.getValue() == "lol");
}

TEST(Misc, recursiveSetSame)
{
    Setting<int> a("/multithread/basic/a");

    a = 0;

    a.connect(
        [&](const int &it) {
            ASSERT_TRUE(it == 1 || it == 2);
            if (it == 1) {
                a = 2;
            }
        },
        false);

    a = 1;
    ASSERT_EQ(a, 2);
}

TEST(Misc, recursiveSetDiff)
{
    Setting<int> a("/multithread/basic/a");
    Setting<int> b("/multithread/basic/b");

    a = 0;
    b = 0;

    a.connect(
        [&](const int &it) {
            ASSERT_TRUE(it == 1 || it == 2);
            if (it == 1) {
                b = 1;
            }
        },
        false);
    b.connect(
        [&](const int &it) {
            ASSERT_EQ(it, 1);
            a = 2;
        },
        false);

    a = 1;
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 1);
}
