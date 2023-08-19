#include "test/common.hpp"

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
