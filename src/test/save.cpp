#include <pajlada/settings.hpp>
#include <pajlada/settings/detail/realpath.hpp>

#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST_CASE("save_int", "[settings][save]")
{
    SettingManager::clear();
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

TEST_CASE("non_symlink", "[settings][save]")
{
    std::string ps = "files/save.not-a-symlink.json";
    REQUIRE(!fs::is_symlink(ps));

    fs_error_code ec;

    auto p1 = fs::path(ps);
    auto p2 = detail::RealPath(p1, ec);

    REQUIRE(!ec);

    REQUIRE(p2 == ps);
}

TEST_CASE("symlink", "[settings][save]")
{
    std::string ps = "files/save.symlink.json";
    REQUIRE(fs::is_symlink(ps));

    SettingManager::clear();
    auto sm = SettingManager::getInstance().get();

    Setting<int>::set("/lol", 10);

    REQUIRE(sm->saveAs(ps.c_str()));

    REQUIRE(fs::is_symlink(ps));
}

TEST_CASE("error_on_recursive_symlink", "[settings][save]")
{
    std::string ps = "files/save.symlink.recursive1.json";
    REQUIRE(fs::is_symlink(ps));

    fs_error_code ec;

    auto finalPath = detail::RealPath(ps, ec);

    REQUIRE(ec);

    REQUIRE(ec.value() ==
            static_cast<int>(std::errc::too_many_symbolic_link_levels));
}

TEST_CASE("error_on_symlink_pointing_to_nonexistant_file", "[settings][save]")
{
    std::string ps = "files/save.symlink.nonexistant.json";
    REQUIRE(fs::is_symlink(ps));

    fs_error_code ec;

    auto finalPath = detail::RealPath(ps, ec);

    REQUIRE(!ec);
}

TEST_CASE("save_backup", "[settings][save]")
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

    REQUIRE(!fs::exists("files/out.backup.json"));

    REQUIRE(sm->save());

    REQUIRE(fs::exists("files/out.backup.json"));
    REQUIRE(!fs::exists("files/out.backup.json.bkp-1"));

    REQUIRE(sm->save());

    REQUIRE(fs::exists("files/out.backup.json"));
    REQUIRE(fs::exists("files/out.backup.json.bkp-1"));
    REQUIRE(!fs::exists("files/out.backup.json.bkp-2"));

    REQUIRE(sm->save());

    REQUIRE(fs::exists("files/out.backup.json"));
    REQUIRE(fs::exists("files/out.backup.json.bkp-1"));
    REQUIRE(fs::exists("files/out.backup.json.bkp-2"));
    REQUIRE(!fs::exists("files/out.backup.json.bkp-3"));

    REQUIRE(sm->save());

    REQUIRE(fs::exists("files/out.backup.json"));
    REQUIRE(fs::exists("files/out.backup.json.bkp-1"));
    REQUIRE(fs::exists("files/out.backup.json.bkp-2"));
    REQUIRE(fs::exists("files/out.backup.json.bkp-3"));
}

TEST_CASE("save_symlink", "[settings][save]")
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;

    std::string bp("files/save.symlink-normal.json");
    std::string tp("files/out.symlink-normal.target.json");

    sm->setPath(bp);

    RemoveFile(tp);

    REQUIRE(fs::is_symlink(bp));
    REQUIRE(!fs::exists(tp));

    {
        Setting<int> setting("/lol", SettingOption::Default, sm);

        setting.setValue(13);
    }

    REQUIRE(sm->save());

    REQUIRE(fs::exists(bp));
    REQUIRE(fs::is_symlink(bp));
    REQUIRE(fs::exists(tp));
}

TEST_CASE("save_backup_symlink", "[settings][save]")
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

    REQUIRE(!fs::exists(tp));
    REQUIRE(!fs::exists(tp1));
    REQUIRE(!fs::exists(tp2));

    auto sm = std::make_shared<SettingManager>();

    sm->setPath(bp);
    sm->setBackupEnabled(true);
    sm->setBackupSlots(2);
    sm->saveMethod = SettingManager::SaveMethod::SaveManually;

    {
        Setting<int> setting("/lol", SettingOption::Default, sm);

        setting.setValue(13);
    }

    REQUIRE(fs::is_symlink(bp));
    REQUIRE(!fs::exists(tp));

    // Save to base file (following the symlink)
    REQUIRE(sm->save());

    REQUIRE(fs::is_symlink(bp));
    REQUIRE(fs::exists(tp));

    REQUIRE(!fs::exists(tp1));

    // Save to base file, should create bkp-1
    REQUIRE(sm->save());

    REQUIRE(fs::exists(tp1));

    REQUIRE(fs::is_symlink(bp2));
    REQUIRE(!fs::exists(tp2));

    // Save to base file, should follow bkp-2 and save a file there
    REQUIRE(sm->save());

    REQUIRE(fs::is_symlink(bp2));
    REQUIRE(fs::exists(tp2));
}
