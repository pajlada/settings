#include "test/class1.hpp"
#include "test/class2.hpp"

#define CATCH_CONFIG_MAIN
#include "libs/catch.hpp"

#include <assert.h>
#include <iostream>

using namespace pajadog;
using namespace pajadog::settings;

TEST_CASE("XD", "xd")
{
    SettingsManager::setPath("hehe.json");

    REQUIRE(Class2::i1.getValue() == 1);
    REQUIRE(Class2::i2.getValue() == 2);
    REQUIRE(Class2::i3.getValue() == 3);
    REQUIRE(Class2::s1.getValue() == "Default string");
    REQUIRE(Class2::f1.getValue() == 1.0101f);
    REQUIRE(Class2::d1.getValue() == 2.0202);

    REQUIRE(SettingsManager::loadFrom("test.json") == true);

    REQUIRE(Class2::i1.getValue() == 2);
    REQUIRE(Class2::i2.getValue() == 3);
    REQUIRE(Class2::i3.getValue() == 4);
    REQUIRE(Class2::s1.getValue() == "Loaded string");
    REQUIRE(Class2::d1.getValue() == 1.0101);
    REQUIRE(Class2::f1.getValue() == 2.0202f);

    Class2::i1.setValue(3);

    REQUIRE(Class2::i1.getValue() == 3);

    Class2::i1 = 4;

    REQUIRE(Class2::i1.getValue() == 4);

    REQUIRE(SettingsManager::saveAs("test2.json") == true);

    SettingsManager::clear();

    REQUIRE(SettingsManager::saveAs("test2.json") == false);
}
