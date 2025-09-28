#include "common.hpp"
#include "pajlada/signals/scoped-connection.hpp"
#include "pajlada/signals/signalholder.hpp"

using namespace pajlada::Settings;

TEST(Signal, Simple)
{
    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<int> a("/simple_signal/a");

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue == 0);

    a.connect(cb, false);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue == 0);

    a = 5;

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue == 5);
}

TEST(Signal, LoadFromFile)
{
    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<int> a("/signal/a");
    Setting<int> b("/signal/b");

    EXPECT_TRUE(a.getValue() == 0);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue == 0);

    a.connect(cb, false);
    b.connect(cb, false);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue == 0);

    a = 5;

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue == 5);

    EXPECT_TRUE(LoadFile("in.signal.json"));

    EXPECT_TRUE(count == 2);
    EXPECT_TRUE(currentValue == 3);
}

TEST(Signal, ScopedConnection)
{
    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    std::vector<std::unique_ptr<pajlada::Signals::ScopedConnection>>
        connections;

    {
        Setting<int> c("/advancedSignals/c");

        EXPECT_TRUE(count == 0);

        c.connect(cb, connections, false);
        c.connect(cb, connections, false);
        c.connect(cb, connections, false);

        EXPECT_TRUE(count == 0);

        // c1, c2, and c3 are active
        c = 1;

        EXPECT_TRUE(count == 3);

        connections.pop_back();

        // c1 and c2 are active
        c = 2;

        EXPECT_TRUE(count == 5);

        connections.clear();

        // No connection is active
        c = 3;

        EXPECT_TRUE(count == 5);
    }
}

TEST(Signal, ScopedConnection2)
{
    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    std::vector<std::unique_ptr<pajlada::Signals::ScopedConnection>>
        connections;

    {
        Setting<int> c("/advancedSignals/c");

        EXPECT_TRUE(count == 0);

        c.connect(cb, connections, false);
        c.connect(cb, connections, false);
        c.connect(cb, connections, false);

        EXPECT_TRUE(count == 0);

        // c1, c2, and c3 are active
        c = 1;

        EXPECT_TRUE(count == 3);

        connections.pop_back();

        {
            Setting<int> c2("/advancedSignals/c");
        }

        // c1 and c2 are active
        c = 2;

        EXPECT_TRUE(count == 5);

        connections.clear();

        // No connection is active
        c = 3;

        EXPECT_TRUE(count == 5);
    }
}

TEST(Signal, ConnectEverything)
{
    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<int> c("/connect/c");
    c = 0;

    {
        pajlada::Signals::ScopedConnection conn;
        c.connect(cb, conn, false);
        c = 1;
        ASSERT_EQ(count, 1);
        ASSERT_EQ(currentValue, 1);
    }
    c = 2;
    ASSERT_EQ(count, 1);
    ASSERT_EQ(currentValue, 1);

    {
        std::list<pajlada::Signals::ScopedConnection> conns;
        c.connect(cb, conns, false);
        c = 3;
        ASSERT_EQ(count, 2);
        ASSERT_EQ(currentValue, 3);
        c.connect(cb, conns, false);
        c = 4;
        ASSERT_EQ(count, 4);
        ASSERT_EQ(currentValue, 4);
    }
    c = 5;
    ASSERT_EQ(count, 4);
    ASSERT_EQ(currentValue, 4);

    {
        std::list<std::unique_ptr<pajlada::Signals::ScopedConnection>> conns;
        c.connect(cb, conns, false);
        c = 6;
        ASSERT_EQ(count, 5);
        ASSERT_EQ(currentValue, 6);
        c.connect(cb, conns, false);
        c = 7;
        ASSERT_EQ(count, 7);
        ASSERT_EQ(currentValue, 7);
    }
    c = 8;
    ASSERT_EQ(count, 7);
    ASSERT_EQ(currentValue, 7);

    // same but with std::vector<>
    {
        std::vector<pajlada::Signals::ScopedConnection> conns;
        c.connect(cb, conns, false);
        c = 9;
        ASSERT_EQ(count, 8);
        ASSERT_EQ(currentValue, 9);
        c.connect(cb, conns, false);
        c = 10;
        ASSERT_EQ(count, 10);
        ASSERT_EQ(currentValue, 10);
    }
    c = 11;
    ASSERT_EQ(count, 10);
    ASSERT_EQ(currentValue, 10);

    {
        std::vector<std::unique_ptr<pajlada::Signals::ScopedConnection>> conns;
        c.connect(cb, conns, false);
        c = 12;
        ASSERT_EQ(count, 11);
        ASSERT_EQ(currentValue, 12);
        c.connect(cb, conns, false);
        c = 13;
        ASSERT_EQ(count, 13);
        ASSERT_EQ(currentValue, 13);
    }
    c = 14;
    ASSERT_EQ(count, 13);
    ASSERT_EQ(currentValue, 13);

    {
        pajlada::Signals::SignalHolder conns;
        c.connect(cb, conns, false);
        c = 15;
        ASSERT_EQ(count, 14);
        ASSERT_EQ(currentValue, 15);
        c.connect(cb, conns, false);
        c = 16;
        ASSERT_EQ(count, 16);
        ASSERT_EQ(currentValue, 16);
    }
    c = 17;
    ASSERT_EQ(count, 16);
    ASSERT_EQ(currentValue, 16);
}
