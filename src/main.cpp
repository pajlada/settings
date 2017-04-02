#include "test/channel.hpp"
#include "test/foo.hpp"

#define CATCH_CONFIG_MAIN
#include "libs/catch.hpp"

#include <assert.h>
#include <iostream>

using namespace pajadog;
using namespace pajadog::settings;

TEST_CASE("Simple static", "xd")
{
    SettingsManager::setPath("hehe.json");

    Channel chHemirt("hemirt");
    REQUIRE(chHemirt.maxMessageLength == 240);

    Channel chPajlada("pajlada");
    REQUIRE(chPajlada.maxMessageLength == 240);

    REQUIRE(Foo::i1.getValue() == 1);
    REQUIRE(Foo::i2.getValue() == 2);
    REQUIRE(Foo::i3.getValue() == 3);
    REQUIRE(Foo::s1.getValue() == "Default string");
    REQUIRE(Foo::f1.getValue() == 1.0101f);
    REQUIRE(Foo::d1.getValue() == 2.0202);

    // Booleans pre-load
    REQUIRE(Foo::b1.getValue() == true);
    REQUIRE(Foo::b2.getValue() == false);
    REQUIRE(Foo::b3.getValue() == true);
    REQUIRE(Foo::b4.getValue() == false);
    REQUIRE(Foo::b5.getValue() == true);
    REQUIRE(Foo::b6.getValue() == false);
    REQUIRE(Foo::b7.getValue() == true);

    REQUIRE(Foo::rootInt1.getValue() == 1);
    REQUIRE(Foo::rootInt2.getValue() == 1);

    REQUIRE(SettingsManager::loadFrom("test.json") == true);

    // Booleans post-load
    REQUIRE(Foo::b1.getValue() == false);  // false
    REQUIRE(Foo::b2.getValue() == true);   // true
    REQUIRE(Foo::b3.getValue() == false);  // 0
    REQUIRE(Foo::b4.getValue() == true);   // 1
    REQUIRE(Foo::b5.getValue() == false);  // 50291 (anything but 1)
    REQUIRE(Foo::b6.getValue() == false);  // nothing loaded
    REQUIRE(Foo::b7.getValue() == true);   // nothing loaded

    REQUIRE(chPajlada.maxMessageLength == 500);

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
