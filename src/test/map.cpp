#include "test/common.hpp"

#include <pajlada/settings.hpp>

using namespace pajlada::Settings;
using namespace std;

#ifdef PAJLADA_BOOST_ANY_SUPPORT
TEST_CASE("Simple Map", "[settings]")
{
    using boost::any_cast;

    Setting<map<string, boost::any>> test("/map");

    REQUIRE(LoadFile("in.simplemap.json"));

    auto myMap = test.getValue();
    REQUIRE(myMap.size() == 3);
    REQUIRE(any_cast<int>(myMap["a"]) == 1);
    REQUIRE(any_cast<string>(myMap["b"]) == "asd");
    REQUIRE(any_cast<double>(myMap["c"]) == 3.14);

    vector<string> keys{"a", "b", "c"};

    REQUIRE(keys == SettingManager::getObjectKeys("/map"));

    REQUIRE(SettingManager::gSaveAs("files/out.simplemap.json"));
}

TEST_CASE("Complex Map", "[settings]")
{
    using boost::any_cast;

    Setting<map<string, boost::any>> test("/map");

    REQUIRE(LoadFile("in.complexmap.json"));

    auto myMap = test.getValue();
    REQUIRE(myMap.size() == 3);
    REQUIRE(any_cast<int>(myMap["a"]) == 5);

    auto innerMap = any_cast<map<string, boost::any>>(myMap["innerMap"]);
    REQUIRE(innerMap.size() == 3);
    REQUIRE(any_cast<int>(innerMap["a"]) == 420);
    REQUIRE(any_cast<int>(innerMap["b"]) == 320);
    REQUIRE(any_cast<double>(innerMap["c"]) == 13.37);

    auto innerArray = any_cast<vector<boost::any>>(myMap["innerArray"]);
    REQUIRE(innerArray.size() == 9);
    REQUIRE(any_cast<int>(innerArray[0]) == 1);
    REQUIRE(any_cast<int>(innerArray[1]) == 2);
    REQUIRE(any_cast<int>(innerArray[2]) == 3);
    REQUIRE(any_cast<int>(innerArray[3]) == 4);
    REQUIRE(any_cast<string>(innerArray[4]) == "testman");
    REQUIRE(any_cast<bool>(innerArray[5]) == true);
    REQUIRE(any_cast<bool>(innerArray[6]) == false);
    REQUIRE(any_cast<double>(innerArray[7]) == 4.20);

    auto innerArrayMap = any_cast<map<string, boost::any>>(innerArray[8]);
    REQUIRE(innerArrayMap.size() == 3);
    REQUIRE(any_cast<int>(innerArrayMap["a"]) == 1);
    REQUIRE(any_cast<int>(innerArrayMap["b"]) == 2);
    REQUIRE(any_cast<int>(innerArrayMap["c"]) == 3);

    REQUIRE(SettingManager::gSaveAs("files/out.complexmap.json"));
}
#endif
