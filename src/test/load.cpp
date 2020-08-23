#include <pajlada/settings.hpp>

#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST_CASE("unicode", "[settings][load]")
{
    SettingManager::clear();

    Setting<int> a("/a", 1);

    REQUIRE(a == 1);

    for (const auto &p : fs::directory_iterator("files/unicode/a")) {
        auto sm = SettingManager::getInstance().get();
        REQUIRE(sm->loadFrom(p) == SettingManager::LoadError::NoError);
    }

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

TEST_CASE("load_symlink", "[settings][load]")
{
    std::string bp("in.symlink.json");

    SettingManager::clear();

    Setting<int> lol("/lol", 1);

    REQUIRE(lol == 1);

    REQUIRE(LoadFile(bp));

    REQUIRE(lol == 10);
}

TEST_CASE("load_relative_symlink", "[settings][load]")
{
    std::string bp("in.relative-symlink.json");

    SettingManager::clear();

    Setting<int> lol("/lol", 1);

    REQUIRE(lol == 1);

    REQUIRE(LoadFile(bp));

    REQUIRE(lol == 10);
}

TEST_CASE("load_absolute_symlink_same_folder", "[settings][load]")
{
    std::string bp("files/in.absolute-symlink-same-folder.json");

    RemoveFile(bp);

    auto cwd = fs::current_path();

    fs::create_symlink(cwd / "files" / "correct.save.save_int.json", bp);

    SettingManager::clear();

    Setting<int> lol("/lol", 1);

    REQUIRE(lol == 1);

    REQUIRE(LoadFile("in.absolute-symlink-same-folder.json"));

    REQUIRE(lol == 10);
}
