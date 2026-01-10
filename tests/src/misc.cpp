#include <gtest/gtest.h>

#include <cassert>
#include <memory>
#include <pajlada/serialize.hpp>
#include <pajlada/settings/setting.hpp>
#include <pajlada/settings/settingdata.hpp>
#include <pajlada/settings/settingmanager.hpp>
#include <string>

#include "channel.hpp"
#include "channelmanager.hpp"

using namespace pajlada::Settings;
using namespace pajlada::test;
using SaveResult = SettingManager::SaveResult;
using SaveMethod = SettingManager::SaveMethod;
using LoadError = SettingManager::LoadError;

TEST(Misc, StdAny)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<std::any> test("/anyTest", sm);
    std::unique_ptr<Setting<std::any>> test2(
        new Setting<std::any>("/anyTest2", sm));

    auto v1 = test.getValue();
    auto v2 = test2->getValue();
}

TEST(Misc, Array)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;

    Setting<int> test1("/array/0/int", sm);
    Setting<int> test2("/array/1/int", sm);
    Setting<int> test3("/array/2/int", sm);

    test1 = 5;
    test2 = 10;
    test3 = 15;

    // This below assertation is not always true
    // It will only be true if the settings above area created with
    // "SaveInitialValue", or if "SaveOnChange" is enabled and the value has
    // been changed
    EXPECT_TRUE(SettingManager::arraySize("/array", sm) == 3);

    EXPECT_EQ(SaveResult::Success, sm->saveAs("files/out.array_test.json"));

    EXPECT_TRUE(SettingManager::arraySize("/array", sm) == 3);
}

TEST(Misc, ArraySize)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;

    EXPECT_EQ(sm->loadFrom("files/in.array_size.json"), LoadError::NoError);

    EXPECT_TRUE(SettingManager::arraySize("/arraySize1", sm) == 1);
    EXPECT_TRUE(SettingManager::arraySize("/arraySize2", sm) == 2);
    EXPECT_TRUE(SettingManager::arraySize("/arraySize3", sm) == 3);
    EXPECT_TRUE(SettingManager::arraySize("/arraySize4", sm) == 4);

    // Not an array
    EXPECT_TRUE(SettingManager::arraySize("/arraySize5", sm) == 0);
}

TEST(Misc, Vector)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<std::vector<int>> test("/vectorTest", sm);

    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/in.vector.json"));

    auto vec = test.getValue();

    EXPECT_TRUE(vec.size() == 3);

    EXPECT_TRUE(vec.at(0) == 5);
    EXPECT_TRUE(vec.at(1) == 10);
    EXPECT_TRUE(vec.at(2) == 15);

    std::vector<int> x = {1, 2, 3};

    test = x;

    EXPECT_EQ(SaveResult::Success, sm->saveAs("files/out.vector.json"));
}

TEST(Misc, ChannelManager)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    ChannelManager manager(sm);

    EXPECT_TRUE(manager.channels.size() == pajlada::test::NUM_CHANNELS);

    for (size_t i = 0; i < manager.channels.size(); ++i) {
        EXPECT_TRUE(manager.channels.at(i).name.getValue() ==
                    "Name not loaded");
    }

    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/channelmanager.json"));

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
    EXPECT_EQ(SaveResult::Success, sm->saveAs("files/out.test3.json"));
    // TODO: Confirm out.test3.json looks correct by comparing it to a manually reviewed file
    EXPECT_TRUE(manager.channels.size() == pajlada::test::NUM_CHANNELS);
}

TEST(Misc, Channel)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Channel chHemirt("hemirt", sm);
    Channel chPajlada("pajlada", sm);

    // Pre-load
    EXPECT_TRUE(chHemirt.maxMessageLength == 240);
    EXPECT_TRUE(chPajlada.maxMessageLength == 240);

    // Load default file
    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/d.channels.json"));

    // Post defaults load
    EXPECT_TRUE(chHemirt.maxMessageLength.getValue() == 200);
    EXPECT_TRUE(chPajlada.maxMessageLength == 240);

    // Load custom file
    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/channels.json"));

    // Post channels load
    EXPECT_TRUE(chHemirt.maxMessageLength == 300);
    EXPECT_TRUE(chPajlada.maxMessageLength == 500);
}

TEST(Misc, LoadFilesInvalidFiles)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    EXPECT_EQ(LoadError::JSONParseError, sm->loadFrom("files/bad-1.json"));
    EXPECT_EQ(LoadError::JSONParseError, sm->loadFrom("files/bad-2.json"));
    EXPECT_EQ(LoadError::JSONParseError, sm->loadFrom("files/bad-3.json"));
    EXPECT_EQ(LoadError::NoError, sm->loadFrom("files/empty.json"));
}

TEST(Misc, NonExistantFiles)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    EXPECT_EQ(LoadError::CannotOpenFile,
              sm->loadFrom("files/test-non-existant-file.json"));
}

TEST(Misc, ValidFiles)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    EXPECT_EQ(LoadError::NoError, sm->loadFrom("files/default.json"));
}

TEST(Misc, Misc)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> test1("/test", sm);
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
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    int v = 69;
    Setting<int> test1("/test", sm);
    EXPECT_TRUE(test1 == 0);
    test1 = 3;
    EXPECT_TRUE(test1 == 3);
    test1 = std::move(v);
    EXPECT_TRUE(test1 == 69);

    std::string v2("lol");
    Setting<std::string> test2("/test", sm);
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
