#include "common.hpp"

using namespace pajlada::Settings;

TEST(Default, Reset)
{
    // No custom default value, not available in the settings file
    Setting<int> noDefault("/noDefault");

    // Custom default value, not available in the settings file
    Setting<int> customDefault("/customDefault", 5);

    // No custom default value, saved in settings file as the same value
    Setting<int> loadedSameNoDefault("/loadedSameNoDefault");

    // No custom default value, saved in settings file as a different value
    Setting<int> loadedDifferentNoDefault("/loadedDifferentNoDefault");

    // Custom default value, saved in settings file as the same value
    Setting<int> loadedSameCustomDefault("/loadedSameCustomDefault", 5);

    // Custom default value, saved in settings file as a different value
    Setting<int> loadedDifferentCustomDefault("/loadedDifferentCustomDefault",
                                              5);

    EXPECT_TRUE(noDefault.getDefaultValue() == 0);
    EXPECT_TRUE(customDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedSameNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedDifferentNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedSameCustomDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    EXPECT_TRUE(noDefault.isDefaultValue());
    EXPECT_TRUE(customDefault.isDefaultValue());
    EXPECT_TRUE(loadedSameNoDefault.isDefaultValue());
    EXPECT_TRUE(loadedDifferentNoDefault.isDefaultValue());
    EXPECT_TRUE(loadedSameCustomDefault.isDefaultValue());
    EXPECT_TRUE(loadedDifferentCustomDefault.isDefaultValue());

    // Before loading
    EXPECT_TRUE(noDefault == 0);
    EXPECT_TRUE(customDefault == 5);
    EXPECT_TRUE(loadedSameNoDefault == 0);
    EXPECT_TRUE(loadedDifferentNoDefault == 0);
    EXPECT_TRUE(loadedSameCustomDefault == 5);
    EXPECT_TRUE(loadedDifferentCustomDefault == 5);

    // Before loading, after resetting all values to their default value
    noDefault.resetToDefaultValue();
    customDefault.resetToDefaultValue();
    loadedSameNoDefault.resetToDefaultValue();
    loadedDifferentNoDefault.resetToDefaultValue();
    loadedSameCustomDefault.resetToDefaultValue();
    loadedDifferentCustomDefault.resetToDefaultValue();

    EXPECT_TRUE(noDefault.getDefaultValue() == 0);
    EXPECT_TRUE(customDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedSameNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedDifferentNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedSameCustomDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    EXPECT_TRUE(noDefault == 0);
    EXPECT_TRUE(customDefault == 5);
    EXPECT_TRUE(loadedSameNoDefault == 0);
    EXPECT_TRUE(loadedDifferentNoDefault == 0);
    EXPECT_TRUE(loadedSameCustomDefault == 5);
    EXPECT_TRUE(loadedDifferentCustomDefault == 5);

    EXPECT_TRUE(LoadFile("in.resettodefault.json"));

    // value does not exist in json file, so should still be same as default
    EXPECT_TRUE(noDefault.isDefaultValue());
    // value does not exist in json file, so should still be same as default
    EXPECT_TRUE(customDefault.isDefaultValue());
    // value exists in json file, and it's the same as the implicit default value
    EXPECT_TRUE(loadedSameNoDefault.isDefaultValue());
    // value exists in json file, but it's different from the implicit default value
    EXPECT_TRUE(!loadedDifferentNoDefault.isDefaultValue());
    // value exists in json file, and it's the same as the explicit default value
    EXPECT_TRUE(loadedSameCustomDefault.isDefaultValue());
    // value exists in json file, but it's different from the explicit default value
    EXPECT_TRUE(!loadedDifferentCustomDefault.isDefaultValue());

    // After loading
    EXPECT_TRUE(noDefault == 0);
    EXPECT_TRUE(customDefault == 5);
    EXPECT_TRUE(loadedSameNoDefault == 0);
    EXPECT_TRUE(loadedDifferentNoDefault == 1);
    EXPECT_TRUE(loadedSameCustomDefault == 5);
    EXPECT_TRUE(loadedDifferentCustomDefault == 6);

    EXPECT_TRUE(noDefault.getDefaultValue() == 0);
    EXPECT_TRUE(customDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedSameNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedDifferentNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedSameCustomDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    // Reset all values to their default values
    noDefault.resetToDefaultValue();
    customDefault.resetToDefaultValue();
    loadedSameNoDefault.resetToDefaultValue();
    loadedDifferentNoDefault.resetToDefaultValue();
    loadedSameCustomDefault.resetToDefaultValue();
    loadedDifferentCustomDefault.resetToDefaultValue();

    EXPECT_TRUE(noDefault.isDefaultValue());
    EXPECT_TRUE(customDefault.isDefaultValue());
    EXPECT_TRUE(loadedSameNoDefault.isDefaultValue());
    EXPECT_TRUE(loadedDifferentNoDefault.isDefaultValue());
    EXPECT_TRUE(loadedSameCustomDefault.isDefaultValue());
    EXPECT_TRUE(loadedDifferentCustomDefault.isDefaultValue());

    EXPECT_TRUE(noDefault.getDefaultValue() == 0);
    EXPECT_TRUE(customDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedSameNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedDifferentNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedSameCustomDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    // After loading, after resetting all values to their default value
    EXPECT_TRUE(noDefault == 0);
    EXPECT_TRUE(customDefault == 5);
    EXPECT_TRUE(loadedSameNoDefault == 0);
    EXPECT_TRUE(loadedDifferentNoDefault == 0);
    EXPECT_TRUE(loadedSameCustomDefault == 5);
    EXPECT_TRUE(loadedDifferentCustomDefault == 5);

    noDefault.setDefaultValue(1);

    EXPECT_TRUE(noDefault == 0);

    EXPECT_TRUE(!noDefault.isDefaultValue());

    noDefault.resetToDefaultValue();

    EXPECT_TRUE(noDefault.isDefaultValue());

    EXPECT_TRUE(noDefault == 1);

    EXPECT_TRUE(noDefault.getDefaultValue() == 1);
    EXPECT_TRUE(customDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedSameNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedDifferentNoDefault.getDefaultValue() == 0);
    EXPECT_TRUE(loadedSameCustomDefault.getDefaultValue() == 5);
    EXPECT_TRUE(loadedDifferentCustomDefault.getDefaultValue() == 5);
}

TEST(Default, UpdateUpdateIteration)
{
    SettingManager::clear();

    Setting<int> setting("/setting", 42);

    EXPECT_EQ(setting.getUpdateIteration(), -1);

    EXPECT_TRUE(LoadFile("empty.json"));
    EXPECT_EQ(setting.getUpdateIteration(), -1);

    EXPECT_EQ(setting.getValue(), 42);
    EXPECT_EQ(setting.getUpdateIteration(), 0);

    setting = 43;
    EXPECT_EQ(setting.getUpdateIteration(), 0);
    EXPECT_EQ(setting.getValue(), 43);
    EXPECT_EQ(setting.getUpdateIteration(), 1);
}
