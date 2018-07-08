#include "libs/catch.hpp"
#include "testhelpers.hpp"

#include <pajlada/settings.hpp>

using namespace pajlada::Settings;

TEST_CASE("RemoveSetting", "[settings][remove]")
{
    Setting<int> a("/rs/a");
    Setting<int> b("/rs/b", 5);
    Setting<int> c("/rs/c");

    // Before loading
    REQUIRE(a == 0);
    REQUIRE(b == 5);
    REQUIRE(c == 0);

    REQUIRE(LoadFile("in.removesetting.json"));

    // After loading
    REQUIRE(a == 5);
    REQUIRE(b == 10);
    REQUIRE(c == 0);

    REQUIRE(SettingManager::gSaveAs("files/out.pre.removesetting.json"));

    REQUIRE(a.remove());

    REQUIRE(SettingManager::gSaveAs("files/out.post.removesetting.json"));

    REQUIRE(!FilesMatch("out.pre.removesetting.json",
                        "out.post.removesetting.json"));
}

TEST_CASE("RemoveNestedSetting", "[settings][remove]")
{
    Setting<int> a("/root/nested/a", 5);
    Setting<int> b("/root/nested/b", 10);
    Setting<int> c("/root/nested/c", 15);
    Setting<int> d("/root/d");
    Setting<int> e("/root/e", 20);

    // Before loading
    REQUIRE(a == 5);
    REQUIRE(b == 10);
    REQUIRE(c == 15);
    REQUIRE(d == 0);
    REQUIRE(e == 20);

    REQUIRE(LoadFile("in.removenestedsetting.json"));

    // After loading
    REQUIRE(a == 6);
    REQUIRE(b == 11);
    REQUIRE(c == 15);
    REQUIRE(d == 0);
    REQUIRE(e == 21);

    REQUIRE(a.isValid());
    REQUIRE(b.isValid());
    REQUIRE(c.isValid());
    REQUIRE(d.isValid());
    REQUIRE(e.isValid());

    REQUIRE(
        SettingManager::gSaveAs("files/out.removenestedsetting.state1.json"));
    REQUIRE(FilesMatch("out.removenestedsetting.state1.json",
                       "in.removenestedsetting.state1.json"));

    REQUIRE(SettingManager::removeSetting("/root/nested/a"));

    REQUIRE(!a.isValid());
    REQUIRE(b.isValid());
    REQUIRE(c.isValid());
    REQUIRE(d.isValid());
    REQUIRE(e.isValid());

    REQUIRE(
        SettingManager::gSaveAs("files/out.removenestedsetting.state2.json"));
    REQUIRE(FilesMatch("out.removenestedsetting.state2.json",
                       "in.removenestedsetting.state2.json"));

    REQUIRE(SettingManager::removeSetting("/root/nested"));

    REQUIRE(!a.isValid());
    REQUIRE(!b.isValid());
    REQUIRE(!c.isValid());
    REQUIRE(d.isValid());
    REQUIRE(e.isValid());

    REQUIRE(
        SettingManager::gSaveAs("files/out.removenestedsetting.state3.json"));
    REQUIRE(FilesMatch("out.removenestedsetting.state3.json",
                       "in.removenestedsetting.state3.json"));

    // lol
}
