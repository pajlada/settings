#include <gtest/gtest.h>
#include <rapidjson/document.h>

#include <pajlada/signals/connection.hpp>

#include "common.hpp"

using namespace pajlada::Settings;
using namespace pajlada::Signals;

TEST(BadInstance, MultipleFiles)
{
    auto sm1 = std::make_shared<SettingManager>();
    auto sm2 = std::make_shared<SettingManager>();

    Setting<int> a("/multi/a", sm1);

    EXPECT_TRUE(a == 0);

    a = 3;

    EXPECT_TRUE(a == 3);

    sm1.reset();

    a.getValue();
}

TEST(BadInstance, Two)
{
    auto sm1 = std::make_shared<SettingManager>();

    Setting<int> a("/multi/a", sm1);

    EXPECT_TRUE(a == 0);

    a = 3;

    EXPECT_TRUE(a == 3);

    auto lockedData = a.getData().lock();

    sm1.reset();

    rapidjson::Value val;

    EXPECT_TRUE(lockedData->marshal<int>(53) == false);
    rapidjson::Document doc;
    EXPECT_FALSE(lockedData->unmarshalJSON(doc));
    EXPECT_TRUE(lockedData->getPath() == "/multi/a");
    EXPECT_TRUE(lockedData->unmarshal<int>() == std::nullopt);

    a.getValue();
}

TEST(MultiInstance, Good1)
{
    auto sm = std::make_shared<SettingManager>();

    EXPECT_TRUE(sm->loadFrom("files/in.multi.json") ==
                SettingManager::LoadError::NoError);

    Setting<int> a("/multi/a", sm);

    bool called = false;

    {
        std::vector<std::unique_ptr<pajlada::Signals::ScopedConnection>>
            managedConnections;

        const bool autoInvoke = true;

        a.connect(
            [&](int value, const SignalArgs &) {
                EXPECT_TRUE(value == 50);
                called = true;
            },
            managedConnections, autoInvoke);
        if (autoInvoke) {
            EXPECT_TRUE(called);
        }
    }

    called = false;
    a.connect([&](int /*value*/) {
        //EXPECT_TRUE(value == 50);  //
        called = true;
    });

    EXPECT_TRUE(called);

    // EXPECT_TRUE(a == 50);
}

TEST(MultiInstance, Good2)
{
    auto sm = std::make_shared<SettingManager>();

    EXPECT_TRUE(sm->loadFrom("files/in.multi.json") ==
                SettingManager::LoadError::NoError);

    Setting<int> a("/multi/a", sm);

    bool called = false;

    {
        std::vector<std::unique_ptr<pajlada::Signals::ScopedConnection>>
            managedConnections;

        const bool autoInvoke = true;

        a.connectJSON(
            [&](const rapidjson::Value &, const SignalArgs &) {
                called = true;  //
            },
            managedConnections, autoInvoke);
        if (autoInvoke) {
            EXPECT_TRUE(called);
        }
    }

    called = false;
    a.connect([&](int value) {
        EXPECT_TRUE(value == 50);  //
        called = true;
    });

    EXPECT_TRUE(called);

    // EXPECT_TRUE(a == 50);
}
