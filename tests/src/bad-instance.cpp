#include <gtest/gtest.h>

#include <pajlada/settings.hpp>
#include <pajlada/signals/connection.hpp>

using namespace pajlada::Settings;
using namespace pajlada::Signals;

TEST(BadInstance, MultipleFiles)
{
    auto sm1 = std::make_shared<SettingManager>();
    auto sm2 = std::make_shared<SettingManager>();

    Setting<int> a("/multi/a", sm1);

    EXPECT_EQ(a, 0);

    a = 3;

    EXPECT_EQ(a, 3);

    sm1.reset();

    a.getValue();
}

TEST(BadInstance, Two)
{
    auto sm1 = std::make_shared<SettingManager>();

    Setting<int> a("/multi/a", sm1);

    EXPECT_EQ(a, 0);

    a = 3;

    EXPECT_EQ(a, 3);

    auto lockedData = a.getData().lock();

    sm1.reset();

    rapidjson::Value val;

    EXPECT_EQ(lockedData->marshal<int>(53), false);
    EXPECT_EQ(lockedData->unmarshalJSON(), nullptr);
    EXPECT_EQ(lockedData->getPath(), "/multi/a");
    EXPECT_EQ(lockedData->unmarshal<int>(), std::nullopt);

    a.getValue();
}

TEST(MultiInstance, Good1)
{
    auto sm = std::make_shared<SettingManager>();

    EXPECT_EQ(sm->loadFrom("files/in.multi.json"),
              SettingManager::LoadError::NoError);

    Setting<int> a("/multi/a", sm);

    bool called = false;

    {
        std::vector<std::unique_ptr<pajlada::Signals::ScopedConnection>>
            managedConnections;

        const bool autoInvoke = true;

        a.connect(
            [&](int value, const SignalArgs &) {
                EXPECT_EQ(value, 50);
                called = true;
            },
            managedConnections, autoInvoke);
        if (autoInvoke) {
            EXPECT_TRUE(called);
        }
    }

    called = false;
    a.connect([&](int /*value*/) {
        //EXPECT_TRUE(value, 50);  //
        called = true;
    });

    EXPECT_TRUE(called);

    // EXPECT_TRUE(a, 50);
}

TEST(MultiInstance, Good2)
{
    auto sm = std::make_shared<SettingManager>();

    EXPECT_EQ(sm->loadFrom("files/in.multi.json"),
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
        EXPECT_EQ(value, 50);  //
        called = true;
    });

    EXPECT_TRUE(called);

    // EXPECT_TRUE(a, 50);
}
