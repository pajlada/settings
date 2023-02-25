#include "test/common.hpp"

using namespace pajlada::Settings;

TEST(Remove, Simple)
{
    Setting<int> a("/rs/a");
    Setting<int> b("/rs/b", 5);
    Setting<int> c("/rs/c");

    // Before loading
    EXPECT_TRUE(a == 0);
    EXPECT_TRUE(b == 5);
    EXPECT_TRUE(c == 0);

    EXPECT_TRUE(LoadFile("in.removesetting.json"));

    // After loading
    EXPECT_TRUE(a == 5);
    EXPECT_TRUE(b == 10);
    EXPECT_TRUE(c == 0);

    EXPECT_TRUE(SettingManager::gSaveAs("files/out.pre.removesetting.json"));

    EXPECT_TRUE(a.remove());

    EXPECT_TRUE(SettingManager::gSaveAs("files/out.post.removesetting.json"));

    EXPECT_TRUE(!FilesMatch("out.pre.removesetting.json",
                            "out.post.removesetting.json"));
}

TEST(Remove, Nested)
{
    Setting<int> a("/root/nested/a", 5);
    Setting<int> b("/root/nested/b", 10);
    Setting<int> c("/root/nested/c", 15);
    Setting<int> d("/root/d");
    Setting<int> e("/root/e", 20);

    // Before loading
    EXPECT_TRUE(a == 5);
    EXPECT_TRUE(b == 10);
    EXPECT_TRUE(c == 15);
    EXPECT_TRUE(d == 0);
    EXPECT_TRUE(e == 20);

    EXPECT_TRUE(LoadFile("in.removenestedsetting.json"));

    // After loading
    EXPECT_TRUE(a == 6);
    EXPECT_TRUE(b == 11);
    EXPECT_TRUE(c == 15);
    EXPECT_TRUE(d == 0);
    EXPECT_TRUE(e == 21);

    EXPECT_TRUE(a.isValid());
    EXPECT_TRUE(b.isValid());
    EXPECT_TRUE(c.isValid());
    EXPECT_TRUE(d.isValid());
    EXPECT_TRUE(e.isValid());

    EXPECT_TRUE(
        SettingManager::gSaveAs("files/out.removenestedsetting.state1.json"));
    EXPECT_TRUE(FilesMatch("out.removenestedsetting.state1.json",
                           "in.removenestedsetting.state1.json"));

    EXPECT_TRUE(SettingManager::removeSetting("/root/nested/a"));

    EXPECT_TRUE(!a.isValid());
    EXPECT_TRUE(b.isValid());
    EXPECT_TRUE(c.isValid());
    EXPECT_TRUE(d.isValid());
    EXPECT_TRUE(e.isValid());

    EXPECT_TRUE(
        SettingManager::gSaveAs("files/out.removenestedsetting.state2.json"));
    EXPECT_TRUE(FilesMatch("out.removenestedsetting.state2.json",
                           "in.removenestedsetting.state2.json"));

    EXPECT_TRUE(SettingManager::removeSetting("/root/nested"));

    EXPECT_TRUE(!a.isValid());
    EXPECT_TRUE(!b.isValid());
    EXPECT_TRUE(!c.isValid());
    EXPECT_TRUE(d.isValid());
    EXPECT_TRUE(e.isValid());

    EXPECT_TRUE(
        SettingManager::gSaveAs("files/out.removenestedsetting.state3.json"));
    EXPECT_TRUE(FilesMatch("out.removenestedsetting.state3.json",
                           "in.removenestedsetting.state3.json"));
}
