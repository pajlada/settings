#include "test/common.hpp"

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

TEST_CASE("Serialize boost::any vector<std::string>", "[settings][serialize]")
{
    using boost::any_cast;

    SettingManager::clear();

    std::vector<std::string> data{"a", "b", "c"};

    Setting<boost::any> a("/a");

    auto rawAny = a.getValue();
    REQUIRE(rawAny.empty());

    a = data;

    rawAny = a.getValue();

    REQUIRE(!rawAny.empty());

    auto vec = any_cast<std::vector<boost::any>>(rawAny);

    REQUIRE(vec.size() == data.size());

    REQUIRE(LoadFile("in.serialize.any.vector.str.json"));

    rawAny = a.getValue();
    vec = any_cast<std::vector<boost::any>>(rawAny);

    REQUIRE(vec.size() == 2);
    REQUIRE(any_cast<std::string>(vec[0]) == "x");
    REQUIRE(any_cast<std::string>(vec[1]) == "D");

    std::vector<std::string> newData{"l", "o", "l", "4HEad"};

    a = newData;

    REQUIRE(SaveFile("out.serialize.any.vector.str.json"));

    REQUIRE(FilesMatch("in.serialize.vector.str.state1.json",
                       "out.serialize.any.vector.str.json"));
}

TEST_CASE("Serialize boost::any vector<boost::any>", "[settings][serialize]")
{
    using boost::any_cast;

    SettingManager::clear();

    std::vector<boost::any> data{"test", 5, 13.37};

    Setting<boost::any> a("/a");

    auto rawAny = a.getValue();
    REQUIRE(rawAny.empty());

    a = data;

    rawAny = a.getValue();
    auto vec = any_cast<std::vector<boost::any>>(rawAny);

    REQUIRE(vec.size() == data.size());

    REQUIRE(vec[0].type().name() == typeid(std::string).name());
    REQUIRE(any_cast<std::string>(vec[0]) == "test");
    REQUIRE(any_cast<int>(vec[1]) == 5);
    REQUIRE(any_cast<double>(vec[2]) == Approx(13.37));

    REQUIRE(LoadFile("in.serialize.any.vector.str.json"));

    rawAny = a.getValue();
    vec = any_cast<std::vector<boost::any>>(rawAny);

    REQUIRE(vec.size() == 2);
    REQUIRE(any_cast<std::string>(vec[0]) == "x");
    REQUIRE(any_cast<std::string>(vec[1]) == "D");

    std::vector<std::string> newData{"l", "o", "l", "4HEad"};

    a = newData;

    REQUIRE(SaveFile("out.serialize.any.vector.str.json"));

    REQUIRE(FilesMatch("in.serialize.vector.str.state1.json",
                       "out.serialize.any.vector.str.json"));
}

TEST_CASE("Serialize int", "[settings][serialize]")
{
    SettingManager::clear();

    int data = 8;

    Setting<int> a("/a");

    SECTION("Before loading a")
    {
        REQUIRE(a == 0);
        REQUIRE(a.getValue() == 0);
        int val = a;
        REQUIRE(val == 0);
    }

    SECTION("Setting")
    {
        a = data;

        REQUIRE(a == data);
        REQUIRE(a.getValue() == data);
        int val = a;
        REQUIRE(val == data);
    }

    REQUIRE(LoadFile("in.serialize.int.json"));

    SECTION("After loading")
    {
        REQUIRE(a == 10);
        REQUIRE(a.getValue() == 10);
        int val = a;
        REQUIRE(val == 10);
    }

    REQUIRE(SaveFile("out.serialize.int.json"));

    REQUIRE(FilesMatch("in.serialize.int.json", "out.serialize.int.json"));
}

TEST_CASE("Serialize bool", "[settings][serialize]")
{
    SettingManager::clear();

    bool data = true;
    bool val;

    Setting<bool> a("/a");

    REQUIRE(a == false);
    REQUIRE(a.getValue() == false);
    val = a;
    REQUIRE(val == false);

    a = data;

    REQUIRE(a == data);
    REQUIRE(a.getValue() == data);
    val = a;
    REQUIRE(val == data);

    a = false;

    REQUIRE(a == false);
    REQUIRE(a.getValue() == false);
    val = a;
    REQUIRE(val == false);

    REQUIRE(LoadFile("in.serialize.bool.json"));
    REQUIRE(a == true);
    REQUIRE(a.getValue() == true);
    val = a;
    REQUIRE(val == true);

    REQUIRE(LoadFile("in.serialize.bool2.json"));
    REQUIRE(a == false);
    REQUIRE(a.getValue() == false);
    val = a;
    REQUIRE(val == false);

    REQUIRE(LoadFile("in.serialize.bool3.json"));
    REQUIRE(a == true);
    REQUIRE(a.getValue() == true);
    val = a;
    REQUIRE(val == true);

    REQUIRE(LoadFile("in.serialize.bool4.json"));
    REQUIRE(a == false);
    REQUIRE(a.getValue() == false);
    val = a;
    REQUIRE(val == false);

    a = true;
    REQUIRE(a == true);
    REQUIRE(a.getValue() == true);
    val = a;
    REQUIRE(val == true);
    REQUIRE(SaveFile("out.serialize.bool.json"));
    REQUIRE(FilesMatch("in.serialize.bool.json", "out.serialize.bool.json"));

    a = false;
    REQUIRE(a == false);
    REQUIRE(a.getValue() == false);
    val = a;
    REQUIRE(val == false);
    REQUIRE(SaveFile("out.serialize.bool.json"));
    REQUIRE(
        FilesMatch("in.serialize.bool.false.json", "out.serialize.bool.json"));
}
