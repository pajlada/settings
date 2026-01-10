#include <gtest/gtest.h>

#include <pajlada/settings.hpp>

#include "common.hpp"

using namespace pajlada::Settings;
using SaveResult = SettingManager::SaveResult;
using SaveMethod = SettingManager::SaveMethod;
using LoadError = SettingManager::LoadError;

namespace fs = std::filesystem;

using LoadError = pajlada::Settings::SettingManager::LoadError;

namespace {

const fs::path PRE = "files/";

}  // namespace

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

TEST(Load, Load)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;
    sm->setPath("thisfiledoesnotexist.json");
    sm->setBackupEnabled(false);

    Setting<int> a("/a", 1, sm);

    EXPECT_TRUE(a == 1);

    EXPECT_EQ(sm->load("files/in.normal.json"), LoadError::NoError);

    EXPECT_TRUE(a == 3);

    a = 2;

    EXPECT_TRUE(a == 2);

    EXPECT_EQ(sm->load(), LoadError::NoError);

    EXPECT_TRUE(a == 3);
}

TEST(Load, LoadFrom)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;
    sm->setPath("thisfiledoesnotexist.json");
    sm->setBackupEnabled(false);

    Setting<int> a("/a", 1, sm);

    EXPECT_TRUE(a == 1);

    EXPECT_EQ(sm->loadFrom("files/in.hastemporary.json"), LoadError::NoError);

    EXPECT_TRUE(a == 4);

    a = 2;

    EXPECT_TRUE(a == 2);

    // Since we used loadFrom, we haven't set the file path to load from so it will try to load from settings.json which shouldn't exist
    EXPECT_EQ(sm->load(), LoadError::CannotOpenFile);
}

TEST(Load, LoadFromWithTemporary)
{
    // This path has a .tmp path, but no
    auto f1 = PRE / "in.hastemporary2.json";
    auto f1tmp = PRE / "in.hastemporary2.json.tmp";

    ASSERT_FALSE(fs::exists(f1));
    ASSERT_TRUE(fs::exists(f1tmp));

    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;
    sm->setPath("thisfiledoesnotexist.json");
    sm->setBackupEnabled(false);
    sm->loadOptions.attemptLoadFromTemporaryFile = true;

    Setting<int> a("/a", 1, sm);

    EXPECT_TRUE(a == 1);

    ASSERT_FALSE(RemoveFile(f1));

    // the main json file does not exist, but it has a .tmp file
    EXPECT_EQ(sm->loadFrom(f1), LoadError::NoError);
    // As a consequence of loading from the .tmp file, we save to the main file
    ASSERT_TRUE(fs::exists(f1));

    EXPECT_TRUE(a == 5);

    a = 2;

    EXPECT_TRUE(a == 2);

    // Since we used loadFrom, we haven't set the file path to load from so it will try to load from settings.json which shouldn't exist
    EXPECT_EQ(sm->load(), LoadError::CannotOpenFile);

    EXPECT_TRUE(RemoveFile("files/in.hastemporary2.json"));
}

TEST(Load, LoadFromWithTemporaryButSaveFails)
{
    // This path has a .tmp path and normal .json file, but the normal .json file has no perms after we call fs::permissions on it
    auto f1 = PRE / "in.hastemporary3.json";
    auto f1tmp = PRE / "in.hastemporary3.json.tmp";

    ASSERT_TRUE(fs::exists(f1));
    ASSERT_TRUE(fs::exists(f1tmp));

    fs::permissions(f1, fs::perms::none);

    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;
    sm->setBackupEnabled(false);
    sm->loadOptions.attemptLoadFromTemporaryFile = true;

    Setting<int> a("/a", 1, sm);

    EXPECT_TRUE(a == 1);

    EXPECT_EQ(sm->loadFrom(f1), LoadError::SavingFromTemporaryFileFailed);

    EXPECT_TRUE(a == 10);

    a = 2;

    EXPECT_TRUE(a == 2);

    fs::permissions(f1, fs::perms::owner_read | fs::perms::owner_write |
                            fs::perms::group_read | fs::perms::others_read);
}
