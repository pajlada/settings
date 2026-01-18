#include <gtest/gtest.h>

#include <pajlada/settings.hpp>

#include "common.hpp"

using namespace pajlada::Settings;
using SaveResult = SettingManager::SaveResult;
using SaveMethod = SettingManager::SaveMethod;
using LoadError = SettingManager::LoadError;

TEST(OptionCompareBeforeSet, Off)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<int> a("/simple_signal/a", sm);

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
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    int count = 0;
    int currentValue = 0;
    auto cb = [&count, &currentValue](const int &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<int> a("/simple_signal/a", SettingOption::CompareBeforeSet, sm);

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
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    int count = 0;
    NonComparableStruct currentValue{
        .a = false,
    };
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<NonComparableStruct> a("/simple_signal/a",
                                   SettingOption::CompareBeforeSet, sm);

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

    // Even though the struct is non-comparable, we use compare the marshalled JSON value - not the C++ type
    EXPECT_TRUE(count == 1);
    EXPECT_TRUE(currentValue.a == true);

    a = NonComparableStruct{
        .a = false,
    };

    EXPECT_TRUE(count == 2);
    EXPECT_TRUE(currentValue.a == false);
}

TEST(OptionCompareBeforeSet, ComparableCustomType)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    int count = 0;
    ComparableStruct currentValue{
        .a = false,
    };
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<ComparableStruct> a("/simple_signal/a",
                                SettingOption::CompareBeforeSet, sm);

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
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    int count = 0;
    std::pair<int, int> currentValue{0, 0};
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<std::pair<int, int>> a("/simple_signal/a",
                                   SettingOption::CompareBeforeSet, sm);

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
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    int count = 0;
    std::vector<int> currentValue{5, 7};
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<std::vector<int>> a("/simple_signal/a",
                                SettingOption::CompareBeforeSet, sm);

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
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    int count = 0;
    std::vector<ComparableStruct> currentValue{};
    auto cb = [&count, &currentValue](const auto &newValue, auto) {
        ++count;
        currentValue = newValue;
    };

    Setting<std::vector<ComparableStruct>> a(
        "/simple_signal/a", SettingOption::CompareBeforeSet, sm);

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
