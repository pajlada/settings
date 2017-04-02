#include "test/foo.hpp"

#define CATCH_CONFIG_MAIN
#include "libs/catch.hpp"

#include <assert.h>
#include <iostream>

using namespace pajadog;
using namespace pajadog::settings;

TEST_CASE("XD", "xd")
{
    SettingsManager::setPath("hehe.json");

    REQUIRE(Foo::i1.getValue() == 1);
    REQUIRE(Foo::i2.getValue() == 2);
    REQUIRE(Foo::i3.getValue() == 3);
    REQUIRE(Foo::s1.getValue() == "Default string");
    REQUIRE(Foo::f1.getValue() == 1.0101f);
    REQUIRE(Foo::d1.getValue() == 2.0202);
    REQUIRE(Foo::rootInt1.getValue() == 1);
    REQUIRE(Foo::rootInt2.getValue() == 1);

    REQUIRE(SettingsManager::loadFrom("test.json") == true);

    REQUIRE(Foo::i1.getValue() == 2);
    REQUIRE(Foo::i2.getValue() == 3);
    REQUIRE(Foo::i3.getValue() == 4);
    REQUIRE(Foo::s1.getValue() == "Loaded string");
    REQUIRE(Foo::d1.getValue() == 1.0101);
    REQUIRE(Foo::f1.getValue() == 2.0202f);
    REQUIRE(Foo::rootInt1.getValue() == 2);
    REQUIRE(Foo::rootInt2.getValue() == 2);

    /*
    Class2::i1.signalValueChanged.connect([](const int &newValue) {
        std::cout << "new value for i1: " << newValue << std::endl;
    });
    */

    Foo::rootInt1 = 3;

    Foo::i1.setValue(3);

    REQUIRE(Foo::i1.getValue() == 3);

    Foo::i1 = 4;

    REQUIRE(Foo::i1.getValue() == 4);

    REQUIRE(SettingsManager::saveAs("test2.json") == true);

    SettingsManager::clear();

    REQUIRE(SettingsManager::saveAs("test2.json") == false);
}
