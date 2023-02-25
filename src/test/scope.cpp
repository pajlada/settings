#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST(Scope, Simple)
{
    Setting<int> a1("/a", 1);

    EXPECT_TRUE(a1 == 1);
    EXPECT_TRUE(a1.getDefaultValue() == 1);

    {
        Setting<int> a2("/a");
        // Because /a is already initialized, we should just load the same
        // shared_ptr that a1 uses

        EXPECT_TRUE(a2 == 0);
        EXPECT_TRUE(a2.getDefaultValue() == 0);

        a2 = 8;

        EXPECT_TRUE(a2 == 8);
        EXPECT_TRUE(a1 == 8);
    }

    EXPECT_TRUE(a1 == 8);

    Setting<int>::set("/a", 10);

    EXPECT_TRUE(a1 == 10);

    {
        Setting<int> a2("/a");

        EXPECT_TRUE(a2 == 10);

        Setting<int>::set("/a", 20);

        EXPECT_TRUE(a2 == 20);
        EXPECT_TRUE(a1 == 20);
    }

    EXPECT_TRUE(a1 == 20);
}
