#include "common.hpp"

using namespace pajlada::Settings;

TEST(OptionCompareBeforeSet, Off)
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

    a = 5;

    EXPECT_TRUE(count == 2);
    EXPECT_TRUE(currentValue == 5);
}

TEST(OptionCompareBeforeSet, On)
{
    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<int> a("/simple_signal/a", SettingOption::CompareBeforeSet);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue == 0);

    a.connect(cb, false);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue == 0);

    a = 5;

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue == 5);

    a = 5;

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue == 5);
}

TEST(OptionCompareBeforeSet, NonComparableCustomType)
{
    int count = 0;
    NonComparableStruct currentValue{
        .a = false,
    };
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<NonComparableStruct> a("/simple_signal/a",
                                   SettingOption::CompareBeforeSet);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue.a == false);

    a.connect(cb, false);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue.a == false);

    a = NonComparableStruct{
        .a = true,
    };

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue.a == true);

    a = NonComparableStruct{
        .a = true,
    };

    // Since the struct is not comparable, no comparison takes place
    EXPECT_TRUE(count == 2);
    EXPECT_TRUE(currentValue.a == true);
}

TEST(OptionCompareBeforeSet, ComparableCustomType)
{
    int count = 0;
    ComparableStruct currentValue{
        .a = false,
    };
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<ComparableStruct> a("/simple_signal/a",
                                SettingOption::CompareBeforeSet);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue.a == false);

    a.connect(cb, false);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue.a == false);

    a = ComparableStruct{
        .a = true,
    };

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue.a == true);

    a = ComparableStruct{
        .a = true,
    };

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue.a == true);

    a = ComparableStruct{
        .a = false,
    };

    EXPECT_TRUE(count == 2);
    EXPECT_TRUE(currentValue.a == false);
}

TEST(OptionCompareBeforeSet, Pair)
{
    int count = 0;
    std::pair<int, int> currentValue{0, 0};
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<std::pair<int, int>> a("/simple_signal/a",
                                   SettingOption::CompareBeforeSet);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue.first == 0);
    EXPECT_TRUE(currentValue.second == 0);

    a.connect(cb, false);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue.first == 0);
    EXPECT_TRUE(currentValue.second == 0);

    a = {1, 2};

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue.first == 1);
    EXPECT_TRUE(currentValue.second == 2);

    a = {1, 2};

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue.first == 1);
    EXPECT_TRUE(currentValue.second == 2);

    a = {2, 3};

    EXPECT_TRUE(count == 2);
    EXPECT_TRUE(currentValue.first == 2);
    EXPECT_TRUE(currentValue.second == 3);
}

TEST(OptionCompareBeforeSet, Vector)
{
    int count = 0;
    std::vector<int> currentValue{5, 7};
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<std::vector<int>> a("/simple_signal/a",
                                SettingOption::CompareBeforeSet);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue.size() == 2);

    a.connect(cb, false);

    EXPECT_TRUE(count == 0);
    EXPECT_TRUE(currentValue.size() == 2);

    a = {5, 7, 3};

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue.size() == 3);

    a = {5, 7, 3};

    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue.size() == 3);
}
