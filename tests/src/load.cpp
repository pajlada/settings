#include <gtest/gtest.h>

#include <pajlada/settings.hpp>

#include "common.hpp"

using namespace pajlada::Settings;
using SaveResult = SettingManager::SaveResult;
using SaveMethod = SettingManager::SaveMethod;
using LoadError = SettingManager::LoadError;

namespace fs = std::filesystem;

TEST(Load, Unicode)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/a", 1, sm);

    EXPECT_TRUE(a == 1);

    for (const auto &p : fs::directory_iterator("files/unicode/a")) {
        EXPECT_EQ(LoadError::NoError, sm->loadFrom(p));
    }

    EXPECT_TRUE(a == 5);
}

TEST(Load, Space)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/a", 1, sm);

    EXPECT_TRUE(a == 1);

    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/load. .json"));

    EXPECT_TRUE(a == 5);
}

TEST(Load, Symlink)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> lol("/lol", 1, sm);

    EXPECT_TRUE(lol == 1);

    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/in.symlink.json"));

    EXPECT_TRUE(lol == 10);
}

TEST(Load, RelativeSymlink)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> lol("/lol", 1, sm);

    EXPECT_TRUE(lol == 1);

    ASSERT_EQ(LoadError::NoError,
              sm->loadFrom("files/in.relative-symlink.json"));

    EXPECT_TRUE(lol == 10);
}

TEST(Load, AbsoluteSymlinkSameFolder)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    std::string bp("files/in.absolute-symlink-same-folder.json");

    RemoveFile(bp);

    auto cwd = fs::current_path();

    fs::create_symlink(cwd / "files" / "correct.save.save_int.json", bp);

    Setting<int> lol("/lol", 1, sm);

    EXPECT_TRUE(lol == 1);

    ASSERT_EQ(LoadError::NoError, sm->loadFrom(bp));

    EXPECT_TRUE(lol == 10);
}
