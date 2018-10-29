#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST_CASE("Scoped settings", "[settings][scope]")
{
    Setting<int> a1("/a", 1);

    REQUIRE(a1 == 1);
    REQUIRE(a1.getDefaultValue() == 1);

    {
        Setting<int> a2("/a");
        // Because /a is already initialized, we should just load the same
        // shared_ptr that a1 uses

        REQUIRE(a2 == 0);
        REQUIRE(a2.getDefaultValue() == 0);

        a2 = 8;

        REQUIRE(a2 == 8);
        REQUIRE(a1 == 8);
    }

    REQUIRE(a1 == 8);

    Setting<int>::set("/a", 10);

    REQUIRE(a1 == 10);

    {
        Setting<int> a2("/a");

        REQUIRE(a2 == 10);

        Setting<int>::set("/a", 20);

        REQUIRE(a2 == 20);
        REQUIRE(a1 == 20);
    }

    REQUIRE(a1 == 20);
}
