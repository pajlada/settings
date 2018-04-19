#include "libs/catch.hpp"
#include "testhelpers.hpp"

#include <pajlada/settings.hpp>

#include <iostream>

using namespace pajlada::Settings;

TEST_CASE("Exception", "[settings][exceptions]")
{
    Setting<int> a("/rs/a");

    REQUIRE(a.isValid());
    REQUIRE(a == 0);

    REQUIRE(a.remove());

    REQUIRE(!a.isValid());

    SECTION("Const ref exception") {
        bool caughtException = false;

        try {
            a = 5;
        } catch (const pajlada::Settings::Exception &e) {
            caughtException = true;
            REQUIRE(e.code == pajlada::Settings::Exception::ExpiredSetting);
        }

        REQUIRE(caughtException);
    }

    SECTION("Copy exception") {
        bool caughtException = false;

        try {
            a = 5;
        } catch (pajlada::Settings::Exception e) {
            caughtException = true;
            REQUIRE(e.code == pajlada::Settings::Exception::ExpiredSetting);
        }

        REQUIRE(caughtException);
    }
}
