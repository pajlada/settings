#include "common.hpp"
#include "pajlada/settings/common.hpp"

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

TEST(OptionCompareBeforeSet, VectorComparableType)
{
    int count = 0;
    std::vector<ComparableStruct> currentValue{};
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<std::vector<ComparableStruct>> a("/simple_signal/a",
                                             SettingOption::CompareBeforeSet);

    EXPECT_EQ(count, 0);
    EXPECT_EQ(currentValue.size(), 0);

    a.connect(cb, false);

    EXPECT_EQ(count, 0);
    EXPECT_EQ(currentValue.size(), 0);

    a = {ComparableStruct{true}};

    EXPECT_EQ(count, 1);
    EXPECT_EQ(currentValue.size(), 1);

    a = {ComparableStruct{true}};

    EXPECT_EQ(count, 1);
    EXPECT_EQ(currentValue.size(), 1);
}

TEST(OptionCompareBeforeSet, VectorNonComparableType)
{
    int count = 0;
    std::vector<NonComparableStruct> currentValue{};
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<std::vector<NonComparableStruct>> a(
        "/simple_signal/a", SettingOption::CompareBeforeSet);

    EXPECT_EQ(count, 0);
    EXPECT_EQ(currentValue.size(), 0);

    a.connect(cb, false);

    EXPECT_EQ(count, 0);
    EXPECT_EQ(currentValue.size(), 0);

    a = {NonComparableStruct{true}};

    EXPECT_EQ(count, 1);
    EXPECT_EQ(currentValue.size(), 1);

    a = {NonComparableStruct{true}};

    // Since the contents of the vector is non-comparable, this will cause an update
    EXPECT_EQ(count, 2);
    EXPECT_EQ(currentValue.size(), 1);
}

TEST(IsEqualityComparable, VectorNonComparableType)
{
    using detail::IsEqualityComparable;

    static_assert(IsEqualityComparable<std::vector<int>>::v);
    static_assert(IsEqualityComparable<std::vector<ComparableStruct>>::v);
    static_assert(
        IsEqualityComparable<std::vector<std::vector<ComparableStruct>>>::v);
    static_assert(!IsEqualityComparable<NonComparableStruct>::v);
    static_assert(!IsEqualityComparable<std::vector<NonComparableStruct>>::v);
    static_assert(!IsEqualityComparable<
                  std::vector<std::vector<NonComparableStruct>>>::v);

    static_assert(IsEqualityComparable<std::list<int>>::v);
    static_assert(IsEqualityComparable<std::list<ComparableStruct>>::v);
    static_assert(
        IsEqualityComparable<std::list<std::list<ComparableStruct>>>::v);
    static_assert(!IsEqualityComparable<NonComparableStruct>::v);
    static_assert(!IsEqualityComparable<std::list<NonComparableStruct>>::v);
    static_assert(
        !IsEqualityComparable<std::list<std::list<NonComparableStruct>>>::v);

    static_assert(IsEqualityComparable<std::pair<int, int>>::v);
    static_assert(IsEqualityComparable<std::pair<ComparableStruct, int>>::v);
    static_assert(IsEqualityComparable<std::pair<int, ComparableStruct>>::v);
    static_assert(
        !IsEqualityComparable<std::pair<NonComparableStruct, int>>::v);
    static_assert(
        !IsEqualityComparable<std::pair<int, NonComparableStruct>>::v);

    static_assert(!IsEqualityComparable<std::any>::v);

    static_assert(
        std::equality_comparable<std::unordered_map<std::string, std::string>>);
    static_assert(std::equality_comparable<
                  std::unordered_map<std::string, ComparableStruct>>);
    static_assert(std::equality_comparable<
                  std::unordered_map<std::string, NonComparableStruct>>);

    static_assert(
        IsEqualityComparable<std::unordered_map<std::string, std::string>>::v);
    static_assert(IsEqualityComparable<
                  std::unordered_map<std::string, ComparableStruct>>::v);
    static_assert(!IsEqualityComparable<
                  std::unordered_map<std::string, NonComparableStruct>>::v);

    static_assert(IsEqualityComparable<std::map<std::string, std::string>>::v);
    static_assert(
        IsEqualityComparable<std::map<std::string, ComparableStruct>>::v);
    static_assert(
        !IsEqualityComparable<std::map<std::string, NonComparableStruct>>::v);

    static_assert(IsEqualityComparable<std::map<std::string, std::string>>::v);
    static_assert(
        IsEqualityComparable<std::map<std::string, ComparableStruct>>::v);
    static_assert(
        !IsEqualityComparable<std::map<std::string, NonComparableStruct>>::v);
}
