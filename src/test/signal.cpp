#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST_CASE("simple_signal", "[signal]")
{
    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<int> a("/simple_signal/a");

    REQUIRE(count == 0);
    REQUIRE(currentValue == 0);

    a.connect(cb, false);

    REQUIRE(count == 0);
    REQUIRE(currentValue == 0);

    a = 5;

    REQUIRE(count == 1);
    REQUIRE(currentValue == 5);
}

TEST_CASE("load_from_file", "[signal]")
{
    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<int> a("/signal/a");
    Setting<int> b("/signal/b");

    REQUIRE(a.getValue() == 0);

    REQUIRE(count == 0);
    REQUIRE(currentValue == 0);

    a.connect(cb, false);
    b.connect(cb, false);

    REQUIRE(count == 0);
    REQUIRE(currentValue == 0);

    a = 5;

    REQUIRE(count == 1);
    REQUIRE(currentValue == 5);

    REQUIRE(LoadFile("in.signal.json"));

    REQUIRE(count == 2);
    REQUIRE(currentValue == 3);
}

TEST_CASE("scoped_connection", "[signal]")
{
    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    vector<std::unique_ptr<pajlada::Signals::ScopedConnection>> connections;

    {
        Setting<int> c("/advancedSignals/c");

        REQUIRE(count == 0);

        c.connect(cb, connections, false);
        c.connect(cb, connections, false);
        c.connect(cb, connections, false);

        REQUIRE(count == 0);

        // c1, c2, and c3 are active
        c = 1;

        REQUIRE(count == 3);

        connections.pop_back();

        // c1 and c2 are active
        c = 2;

        REQUIRE(count == 5);

        connections.clear();

        // No connection is active
        c = 3;

        REQUIRE(count == 5);
    }
}

TEST_CASE("scoped_connection2", "[signal]")
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

        REQUIRE(count == 0);

        c.connect(cb, connections, false);
        c.connect(cb, connections, false);
        c.connect(cb, connections, false);

        REQUIRE(count == 0);

        // c1, c2, and c3 are active
        c = 1;

        REQUIRE(count == 3);

        connections.pop_back();

        {
            Setting<int> c2("/advancedSignals/c");
        }

        // c1 and c2 are active
        c = 2;

        REQUIRE(count == 5);

        connections.clear();

        // No connection is active
        c = 3;

        REQUIRE(count == 5);
    }
}
