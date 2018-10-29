#include "test/common.hpp"

#include <pajlada/settings.hpp>

using namespace pajlada::Settings;
using namespace std;

TEST_CASE("save_int", "[settings][save]")
{
    // This will not be part of the final file, since we clear the settings right afterwards
    // Setting<int>::set("/asd", 5);

    // SettingManager::clear();

    Setting<int>::set("/lol", 10);

    REQUIRE(SaveFile("out.save.save_int.json"));

    REQUIRE(FilesMatch("out.save.save_int.json", "correct.save.save_int.json"));
}

TEST_CASE("save_do_not_write_to_json", "[settings][save]")
{
    Setting<int>::set("/asd", 5, SettingOption::DoNotWriteToJSON);

    Setting<int>::set("/lol", 10);

    REQUIRE(SaveFile("out.save.save_int.json"));

    REQUIRE(FilesMatch("out.save.save_int.json", "correct.save.save_int.json"));
}
