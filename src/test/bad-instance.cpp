#include "test/common.hpp"

#include <pajlada/signals/connection.hpp>

using namespace pajlada::Settings;
using namespace pajlada::Signals;
using namespace std;

TEST_CASE("Multiple files", "[settings][multi_instance][error]")
{
    auto sm1 = std::make_shared<SettingManager>();
    auto sm2 = std::make_shared<SettingManager>();

    Setting<int> a("/multi/a", sm1);

    REQUIRE(a == 0);

    a = 3;

    REQUIRE(a == 3);

    sm1.reset();

    a.getValue();
}

TEST_CASE("Bad 2", "[settings][multi_instance][error]")
{
    auto sm1 = std::make_shared<SettingManager>();

    Setting<int> a("/multi/a", sm1);

    REQUIRE(a == 0);

    a = 3;

    REQUIRE(a == 3);

    auto lockedData = a.getData().lock();

    sm1.reset();

    rapidjson::Value val;
    ValueResult<int> p{std::nullopt, -1};

    REQUIRE(lockedData->marshal<int>(53) == false);
    REQUIRE(lockedData->unmarshalJSON() == nullptr);
    REQUIRE(lockedData->getPath() == "/multi/a");
    REQUIRE(lockedData->unmarshal<int>() == p);

    a.getValue();
}

TEST_CASE("Good 1", "[settings][multi_instance]")
{
    auto sm = std::make_shared<SettingManager>();

    REQUIRE(sm->loadFrom("files/in.multi.json") ==
            SettingManager::LoadError::NoError);

    Setting<int> a("/multi/a", sm);

    bool called = false;

    {
        std::vector<std::unique_ptr<pajlada::Signals::ScopedConnection>>
            managedConnections;

        const bool autoInvoke = true;

        a.connect(
            [&](int value, const SignalArgs &) {
                REQUIRE(value == 50);
                called = true;
            },
            managedConnections, autoInvoke);
        if (autoInvoke) {
            REQUIRE(called);
        }
    }

    called = false;
    a.connect([&](int value) {
        //REQUIRE(value == 50);  //
        called = true;
    });

    REQUIRE(called);

    // REQUIRE(a == 50);
}

TEST_CASE("Good 2", "[settings][multi_instance]")
{
    auto sm = std::make_shared<SettingManager>();

    REQUIRE(sm->loadFrom("files/in.multi.json") ==
            SettingManager::LoadError::NoError);

    Setting<int> a("/multi/a", sm);

    bool called = false;

    {
        std::vector<std::unique_ptr<pajlada::Signals::ScopedConnection>>
            managedConnections;

        const bool autoInvoke = true;

        a.connectJSON(
            [&](const rapidjson::Value &v, const SignalArgs &) {
                called = true;  //
            },
            managedConnections, autoInvoke);
        if (autoInvoke) {
            REQUIRE(called);
        }
    }

    called = false;
    a.connect([&](int value) {
        REQUIRE(value == 50);  //
        called = true;
    });

    REQUIRE(called);

    // REQUIRE(a == 50);
}
