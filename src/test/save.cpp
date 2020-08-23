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
