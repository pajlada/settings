#include <pajlada/settings.hpp>

#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST_CASE("unicode", "[settings][load]")
{
    SettingManager::clear();

    Setting<int> a("/a", 1);

    REQUIRE(a == 1);

    REQUIRE(LoadFile("load.ö.json"));

    REQUIRE(a == 5);
}

TEST_CASE("space", "[settings][load]")
{
    SettingManager::clear();

    Setting<int> a("/a", 1);

    REQUIRE(a == 1);

    REQUIRE(LoadFile("load. .json"));

    REQUIRE(a == 5);
}
