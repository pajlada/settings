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
    EXPECT_EQ(a, 0);
    EXPECT_EQ(b, 5);
    EXPECT_EQ(c, 0);

    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/in.removesetting.json"));

    // After loading
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 10);
    EXPECT_EQ(c, 0);

    ASSERT_EQ(SaveResult::Success,
              sm->saveAs("files/out.pre.removesetting.json"));

    EXPECT_TRUE(sm->removeSetting(a.getPath()));

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/out.post.removesetting.json"));

    AssertFilesDontMatch("out.pre.removesetting.json",
                         "out.post.removesetting.json");
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
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 10);
    EXPECT_EQ(c, 15);
    EXPECT_EQ(d, 0);
    EXPECT_EQ(e, 20);

    ASSERT_EQ(LoadError::NoError,
              sm->loadFrom("files/in.removenestedsetting.json"));

    // After loading
    EXPECT_EQ(a, 6);
    EXPECT_EQ(b, 11);
    EXPECT_EQ(c, 15);
    EXPECT_EQ(d, 0);
    EXPECT_EQ(e, 21);

    EXPECT_TRUE(a.isValid());
    EXPECT_TRUE(b.isValid());
    EXPECT_TRUE(c.isValid());
    EXPECT_TRUE(d.isValid());
    EXPECT_TRUE(e.isValid());

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/out.removenestedsetting.state1.json"));

    AssertFilesMatch("in.removenestedsetting.state1.json",
                     "out.removenestedsetting.state1.json");

    EXPECT_TRUE(sm->removeSetting("/root/nested/a"));

    EXPECT_TRUE(!a.isValid());
    EXPECT_TRUE(b.isValid());
    EXPECT_TRUE(c.isValid());
    EXPECT_TRUE(d.isValid());
    EXPECT_TRUE(e.isValid());

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/out.removenestedsetting.state2.json"));
    AssertFilesMatch("in.removenestedsetting.state2.json",
                     "out.removenestedsetting.state2.json");

    EXPECT_TRUE(sm->removeSetting("/root/nested"));

    EXPECT_TRUE(!a.isValid());
    EXPECT_TRUE(!b.isValid());
    EXPECT_TRUE(!c.isValid());
    EXPECT_TRUE(d.isValid());
    EXPECT_TRUE(e.isValid());

    EXPECT_EQ(SaveResult::Success,
              sm->saveAs("files/out.removenestedsetting.state3.json"));
    AssertFilesMatch("in.removenestedsetting.state3.json",
                     "out.removenestedsetting.state3.json");
}

TEST(Remove, MultipleCalls)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/rs/a", sm);
    Setting<int> b("/rs/b", 5, sm);
    Setting<int> c("/rs/c", sm);

    // Before loading
    EXPECT_EQ(a, 0);
    EXPECT_EQ(b, 5);
    EXPECT_EQ(c, 0);

    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/in.removesetting.json"));

    // After loading
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 10);
    EXPECT_EQ(c, 0);

    // C was not defined, so removeSetting should return false
    EXPECT_FALSE(sm->removeSetting(c.getPath()));

    // A was defined, so removeSetting should return true only the first time
    EXPECT_TRUE(sm->removeSetting(a.getPath()));
    EXPECT_FALSE(sm->removeSetting(a.getPath()));
}

TEST(Remove, Invalidated)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/a", sm);

    ASSERT_EQ(a, 0);

    a = 3;

    ASSERT_EQ(a, 3);

    ASSERT_TRUE(a.isValid());

    ASSERT_TRUE(sm->removeSetting(a.getPath()));

    // After the setting is removed, it can no longer be used
    ASSERT_FALSE(a.isValid());
}
