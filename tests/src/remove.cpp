#include <gtest/gtest.h>

#include <pajlada/settings.hpp>

#include "common.hpp"

using namespace pajlada::Settings;
using SaveResult = SettingManager::SaveResult;
using SaveMethod = SettingManager::SaveMethod;
using LoadError = SettingManager::LoadError;

TEST(Remove, Simple)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/rs/a", sm);
    Setting<int> b("/rs/b", 5, sm);
    Setting<int> c("/rs/c", sm);

    // Before loading
    EXPECT_TRUE(a == 0);
    EXPECT_TRUE(b == 5);
    EXPECT_TRUE(c == 0);

    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/in.removesetting.json"));

    // After loading
    EXPECT_TRUE(a == 5);
    EXPECT_TRUE(b == 10);
    EXPECT_TRUE(c == 0);

    ASSERT_EQ(SaveResult::Success,
              sm->saveAs("files/out.pre.removesetting.json"));

    EXPECT_TRUE(sm->removeSetting(a.getPath()));

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/out.post.removesetting.json"));

    EXPECT_TRUE(!FilesMatch("out.pre.removesetting.json",
                            "out.post.removesetting.json"));
}

TEST(Remove, Nested)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/root/nested/a", 5, sm);
    Setting<int> b("/root/nested/b", 10, sm);
    Setting<int> c("/root/nested/c", 15, sm);
    Setting<int> d("/root/d", sm);
    Setting<int> e("/root/e", 20, sm);

    // Before loading
    EXPECT_TRUE(a == 5);
    EXPECT_TRUE(b == 10);
    EXPECT_TRUE(c == 15);
    EXPECT_TRUE(d == 0);
    EXPECT_TRUE(e == 20);

    ASSERT_EQ(LoadError::NoError,
              sm->loadFrom("files/in.removenestedsetting.json"));

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

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/out.removenestedsetting.state1.json"));
    EXPECT_TRUE(FilesMatch("out.removenestedsetting.state1.json",
                           "in.removenestedsetting.state1.json"));

    EXPECT_TRUE(sm->removeSetting("/root/nested/a"));

    EXPECT_TRUE(!a.isValid());
    EXPECT_TRUE(b.isValid());
    EXPECT_TRUE(c.isValid());
    EXPECT_TRUE(d.isValid());
    EXPECT_TRUE(e.isValid());

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/out.removenestedsetting.state2.json"));
    EXPECT_TRUE(FilesMatch("out.removenestedsetting.state2.json",
                           "in.removenestedsetting.state2.json"));

    EXPECT_TRUE(sm->removeSetting("/root/nested"));

    EXPECT_TRUE(!a.isValid());
    EXPECT_TRUE(!b.isValid());
    EXPECT_TRUE(!c.isValid());
    EXPECT_TRUE(d.isValid());
    EXPECT_TRUE(e.isValid());

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/out.removenestedsetting.state3.json"));
    EXPECT_TRUE(FilesMatch("out.removenestedsetting.state3.json",
                           "in.removenestedsetting.state3.json"));
}
