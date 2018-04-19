#include "libs/catch.hpp"
#include "testhelpers.hpp"

#include <pajlada/settings.hpp>

#include <iostream>

using namespace pajlada::Settings;

TEST_CASE("Serialize vector", "[settings][serialize]")
{
    std::vector<std::string> data{"a", "b", "c"};
    Setting<std::vector<std::string>> a("/a");

    SECTION("Before loading")
    {
        auto vec = a.getValue();

        REQUIRE(vec.size() == 0);
    }

    SECTION("Setting")
    {
        a = data;

        auto vec = a.getValue();

        REQUIRE(vec.size() == data.size());
    }

    REQUIRE(LoadFile("in.serialize.vector.str.json"));

    SECTION("After loading")
    {
        auto vec = a.getValue();

        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == "x");
        REQUIRE(vec[1] == "D");
    }

    std::vector<std::string> newData{"l", "o", "l", "4HEad"};

    a = newData;

    REQUIRE(SaveFile("out.serialize.vector.str.json"));

    REQUIRE(FilesMatch("in.serialize.vector.str.state1.json",
                       "out.serialize.vector.str.json"));
}
