#include <pajlada/settings.hpp>

#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST(Load, Unicode)
{
    SettingManager::clear();

    Setting<int> a("/a", 1);

    EXPECT_TRUE(a == 1);

    for (const auto &p : fs::directory_iterator("files/unicode/a")) {
        auto sm = SettingManager::getInstance().get();
        EXPECT_TRUE(sm->loadFrom(p) == SettingManager::LoadError::NoError);
    }

    EXPECT_TRUE(a == 5);
}

TEST(Load, Space)
{
    SettingManager::clear();

    Setting<int> a("/a", 1);

    EXPECT_TRUE(a == 1);

    EXPECT_TRUE(LoadFile("load. .json"));

    EXPECT_TRUE(a == 5);
}

TEST(Load, Symlink)
{
    std::string bp("in.symlink.json");

    SettingManager::clear();

    Setting<int> lol("/lol", 1);

    EXPECT_TRUE(lol == 1);

    EXPECT_TRUE(LoadFile(bp));

    EXPECT_TRUE(lol == 10);
}

TEST(Load, RelativeSymlink)
{
    std::string bp("in.relative-symlink.json");

    SettingManager::clear();

    Setting<int> lol("/lol", 1);

    EXPECT_TRUE(lol == 1);

    EXPECT_TRUE(LoadFile(bp));

    EXPECT_TRUE(lol == 10);
}

TEST(Load, AbsoluteSymlinkSameFolder)
{
    std::string bp("files/in.absolute-symlink-same-folder.json");

    RemoveFile(bp);

    auto cwd = fs::current_path();

    fs::create_symlink(cwd / "files" / "correct.save.save_int.json", bp);

    SettingManager::clear();

    Setting<int> lol("/lol", 1);

    EXPECT_TRUE(lol == 1);

    EXPECT_TRUE(LoadFile("in.absolute-symlink-same-folder.json"));

    EXPECT_TRUE(lol == 10);
}
