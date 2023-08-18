
#include <atomic>

#include "pajlada/settings/settinglistener.hpp"
#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace pajlada;

TEST(Listener, simple)
{
    Setting<int> a("/listener/simple/a");
    Setting<int> b("/listener/simple/b");

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
    Setting<int> a("/listener/autoinvoke/a");
    Setting<int> b("/listener/autoinvoke/b");

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
    Setting<int> a("/listener/manual-invoke/a");
    Setting<int> b("/listener/manual-invoke/b");

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
    Setting<int> a("/listener/reset-callback/a");
    Setting<int> b("/listener/reset-callback/b");

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
    Setting<int> a("/listener/empty-callback/a");
    Setting<int> b("/listener/empty-callback/b");

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
