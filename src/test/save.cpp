#include <gtest/gtest.h>

#include <pajlada/settings.hpp>
#include <pajlada/settings/detail/realpath.hpp>

#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST(Save, Int)
{
    SettingManager::clear();
    // This will not be part of the final file, since we clear the settings right afterwards
    // Setting<int>::set("/asd", 5);

    // SettingManager::clear();

    Setting<int>::set("/lol", 10);

    EXPECT_TRUE(SaveFile("out.save.save_int.json"));

    EXPECT_TRUE(
        FilesMatch("out.save.save_int.json", "correct.save.save_int.json"));
}

TEST(Save, DoNotWriteToJSON)
{
    Setting<int>::set("/asd", 5, SettingOption::DoNotWriteToJSON);

    Setting<int>::set("/lol", 10);

    EXPECT_TRUE(SaveFile("out.save.save_int.json"));

    EXPECT_TRUE(
        FilesMatch("out.save.save_int.json", "correct.save.save_int.json"));
}

TEST(Save, NonSymlink)
{
    std::string ps = "files/save.not-a-symlink.json";
    EXPECT_TRUE(!fs::is_symlink(ps));

    fs_error_code ec;

    auto p1 = fs::path(ps);
    auto p2 = detail::RealPath(p1, ec);

    EXPECT_TRUE(!ec);

    EXPECT_TRUE(p2 == ps);
}

TEST(Save, Symlink)
{
    std::string ps = "files/save.symlink.json";
    EXPECT_TRUE(fs::is_symlink(ps));

    SettingManager::clear();
    auto sm = SettingManager::getInstance().get();

    Setting<int>::set("/lol", 10);

    EXPECT_TRUE(sm->saveAs(ps.c_str()));

    EXPECT_TRUE(fs::is_symlink(ps));
}

TEST(Save, ErrorOnRecursiveSymlink)
{
    std::string ps = "files/save.symlink.recursive1.json";
    EXPECT_TRUE(fs::is_symlink(ps));

    fs_error_code ec;

    auto finalPath = detail::RealPath(ps, ec);

    EXPECT_TRUE(ec);

    EXPECT_TRUE(ec.value() ==
                static_cast<int>(std::errc::too_many_symbolic_link_levels));
}

TEST(Save, ErrorOnSymlinkPointingToNonexistantFile)
{
    std::string ps = "files/save.symlink.nonexistant.json";
    EXPECT_TRUE(fs::is_symlink(ps));

    fs_error_code ec;

    auto finalPath = detail::RealPath(ps, ec);

    EXPECT_TRUE(!ec);
}

TEST(Save, Backup)
{
    auto sm = std::make_shared<SettingManager>();

    sm->setPath("files/out.backup.json");

    sm->saveMethod = SettingManager::SaveMethod::SaveManually;

    RemoveFile("files/out.backup.json");
    RemoveFile("files/out.backup.json.bkp-1");
    RemoveFile("files/out.backup.json.bkp-2");
    RemoveFile("files/out.backup.json.bkp-3");

    sm->setBackupEnabled(true);
    sm->setBackupSlots(3);

    {
        Setting<int> setting("/lol", SettingOption::Default, sm);

        setting.setValue(13);
    }

    EXPECT_TRUE(!fs::exists("files/out.backup.json"));

    EXPECT_TRUE(sm->save());

    EXPECT_TRUE(fs::exists("files/out.backup.json"));
    EXPECT_TRUE(!fs::exists("files/out.backup.json.bkp-1"));

    EXPECT_TRUE(sm->save());

    EXPECT_TRUE(fs::exists("files/out.backup.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.json.bkp-1"));
    EXPECT_TRUE(!fs::exists("files/out.backup.json.bkp-2"));

    EXPECT_TRUE(sm->save());

    EXPECT_TRUE(fs::exists("files/out.backup.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.json.bkp-1"));
    EXPECT_TRUE(fs::exists("files/out.backup.json.bkp-2"));
    EXPECT_TRUE(!fs::exists("files/out.backup.json.bkp-3"));

    EXPECT_TRUE(sm->save());

    EXPECT_TRUE(fs::exists("files/out.backup.json"));
    EXPECT_TRUE(fs::exists("files/out.backup.json.bkp-1"));
    EXPECT_TRUE(fs::exists("files/out.backup.json.bkp-2"));
    EXPECT_TRUE(fs::exists("files/out.backup.json.bkp-3"));
}

TEST(Save, SaveSymlink)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;

    std::string bp("files/save.symlink-normal.json");
    std::string tp("files/out.symlink-normal.target.json");

    sm->setPath(bp);

    RemoveFile(tp);

    EXPECT_TRUE(fs::is_symlink(bp));
    EXPECT_TRUE(!fs::exists(tp));

    {
        Setting<int> setting("/lol", SettingOption::Default, sm);

        setting.setValue(13);
    }

    EXPECT_TRUE(sm->save());

    EXPECT_TRUE(fs::exists(bp));
    EXPECT_TRUE(fs::is_symlink(bp));
    EXPECT_TRUE(fs::exists(tp));
}

TEST(Save, SaveBackupSymlink)
{
    // In this scenario:
    // The base file is a symlink
    // bkp-1 is not a symlink
    // bkp-2 is a symlink
    std::string bp("files/save.symlink-backup.json");
    std::string tp("files/out.symlink-backup.target.json");

    std::string tp1("files/save.symlink-backup.json.bkp-1");

    std::string bp2("files/save.symlink-backup.json.bkp-2");
    std::string tp2("files/out.symlink-backup.target.json.bkp-2");

    // Start from a clean slate - we should only have symlinks at the start
    RemoveFile(tp);
    RemoveFile(tp1);
    RemoveFile(tp2);

    EXPECT_TRUE(!fs::exists(tp));
    EXPECT_TRUE(!fs::exists(tp1));
    EXPECT_TRUE(!fs::exists(tp2));

    auto sm = std::make_shared<SettingManager>();

    sm->setPath(bp);
    sm->setBackupEnabled(true);
    sm->setBackupSlots(2);
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;

    {
        Setting<int> setting("/lol", SettingOption::Default, sm);

        setting.setValue(13);
    }

    EXPECT_TRUE(fs::is_symlink(bp));
    EXPECT_TRUE(!fs::exists(tp));

    // Save to base file (following the symlink)
    EXPECT_TRUE(sm->save());

    EXPECT_TRUE(fs::is_symlink(bp));
    EXPECT_TRUE(fs::exists(tp));

    EXPECT_TRUE(!fs::exists(tp1));

    // Save to base file, should create bkp-1
    EXPECT_TRUE(sm->save());

    EXPECT_TRUE(fs::exists(tp1));

    EXPECT_TRUE(fs::is_symlink(bp2));
    EXPECT_TRUE(!fs::exists(tp2));

    // Save to base file, should follow bkp-2 and save a file there
    EXPECT_TRUE(sm->save());

    EXPECT_TRUE(fs::is_symlink(bp2));
    EXPECT_TRUE(fs::exists(tp2));
}
