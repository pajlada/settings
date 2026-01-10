#include <gtest/gtest.h>

#include <pajlada/settings.hpp>

#include "pajlada/settings/settinglistener.hpp"

using namespace pajlada::Settings;
using namespace pajlada;
using SaveResult = SettingManager::SaveResult;
using SaveMethod = SettingManager::SaveMethod;
using LoadError = SettingManager::LoadError;

TEST(Listener, simple)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/listener/simple/a", sm);
    Setting<int> b("/listener/simple/b", sm);

    size_t invocations = 0;
    SettingListener listener([&] { invocations++; });

    ASSERT_EQ(invocations, 0);
    listener.addSetting(a);
    ASSERT_EQ(invocations, 0);
    listener.addSetting(b);
    ASSERT_EQ(invocations, 0);

    a = 42;
    ASSERT_EQ(invocations, 1);
    b = 42;
    ASSERT_EQ(invocations, 2);
    a = 42;
    ASSERT_EQ(invocations, 3);
    a = 1;
    ASSERT_EQ(invocations, 4);
}

TEST(Listener, autoinvoke)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/listener/autoinvoke/a", sm);
    Setting<int> b("/listener/autoinvoke/b", sm);

    size_t invocations = 0;
    SettingListener listener([&] { invocations++; });

    ASSERT_EQ(invocations, 0);
    listener.addSetting(a, true);
    ASSERT_EQ(invocations, 1);
    listener.addSetting(b);
    ASSERT_EQ(invocations, 1);

    a = 42;
    ASSERT_EQ(invocations, 2);
    b = 42;
    ASSERT_EQ(invocations, 3);
    a = 42;
    ASSERT_EQ(invocations, 4);
    a = 1;
    ASSERT_EQ(invocations, 5);
}

TEST(Listener, manualInvoke)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/listener/manual-invoke/a", sm);
    Setting<int> b("/listener/manual-invoke/b", sm);

    size_t invocations = 0;
    SettingListener listener([&] { invocations++; });

    ASSERT_EQ(invocations, 0);
    listener.addSetting(a);
    ASSERT_EQ(invocations, 0);
    listener.addSetting(b);
    ASSERT_EQ(invocations, 0);

    a = 42;
    ASSERT_EQ(invocations, 1);
    listener.invoke();
    ASSERT_EQ(invocations, 2);
    b = 42;
    ASSERT_EQ(invocations, 3);
}

TEST(Listener, resetCallback)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/listener/reset-callback/a", sm);
    Setting<int> b("/listener/reset-callback/b", sm);

    size_t invocations = 0;
    SettingListener listener([&] { invocations++; });

    ASSERT_EQ(invocations, 0);
    listener.addSetting(a);
    ASSERT_EQ(invocations, 0);
    listener.addSetting(b);
    ASSERT_EQ(invocations, 0);

    a = 42;
    ASSERT_EQ(invocations, 1);
    listener.resetCB();
    ASSERT_EQ(invocations, 1);
    b = 42;
    a = 42;
    listener.invoke();
    ASSERT_EQ(invocations, 1);
}

TEST(Listener, emptyCallback)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/listener/empty-callback/a", sm);
    Setting<int> b("/listener/empty-callback/b", sm);

    SettingListener listener;

    listener.addSetting(a);
    listener.addSetting(b);

    b = 42;
    a = 42;
    listener.invoke();

    listener.resetCB();
    b = 42;
    a = 42;
    listener.invoke();
}
