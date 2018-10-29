#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST_CASE("ResetToDefault", "[settings][default]")
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

    REQUIRE(noDefault.getDefaultValue() == 0);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    REQUIRE(noDefault.isDefaultValue());
    REQUIRE(customDefault.isDefaultValue());
    REQUIRE(loadedSameNoDefault.isDefaultValue());
    REQUIRE(loadedDifferentNoDefault.isDefaultValue());
    REQUIRE(loadedSameCustomDefault.isDefaultValue());
    REQUIRE(loadedDifferentCustomDefault.isDefaultValue());

    // Before loading
    REQUIRE(noDefault == 0);
    REQUIRE(customDefault == 5);
    REQUIRE(loadedSameNoDefault == 0);
    REQUIRE(loadedDifferentNoDefault == 0);
    REQUIRE(loadedSameCustomDefault == 5);
    REQUIRE(loadedDifferentCustomDefault == 5);

    // Before loading, after resetting all values to their default value
    noDefault.resetToDefaultValue();
    customDefault.resetToDefaultValue();
    loadedSameNoDefault.resetToDefaultValue();
    loadedDifferentNoDefault.resetToDefaultValue();
    loadedSameCustomDefault.resetToDefaultValue();
    loadedDifferentCustomDefault.resetToDefaultValue();

    REQUIRE(noDefault.getDefaultValue() == 0);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    REQUIRE(noDefault == 0);
    REQUIRE(customDefault == 5);
    REQUIRE(loadedSameNoDefault == 0);
    REQUIRE(loadedDifferentNoDefault == 0);
    REQUIRE(loadedSameCustomDefault == 5);
    REQUIRE(loadedDifferentCustomDefault == 5);

    REQUIRE(LoadFile("in.resettodefault.json"));

    // value does not exist in json file, so should still be same as default
    REQUIRE(noDefault.isDefaultValue());
    // value does not exist in json file, so should still be same as default
    REQUIRE(customDefault.isDefaultValue());
    // value exists in json file, and it's the same as the implicit default value
    REQUIRE(loadedSameNoDefault.isDefaultValue());
    // value exists in json file, but it's different from the implicit default value
    REQUIRE(!loadedDifferentNoDefault.isDefaultValue());
    // value exists in json file, and it's the same as the explicit default value
    REQUIRE(loadedSameCustomDefault.isDefaultValue());
    // value exists in json file, but it's different from the explicit default value
    REQUIRE(!loadedDifferentCustomDefault.isDefaultValue());

    // After loading
    REQUIRE(noDefault == 0);
    REQUIRE(customDefault == 5);
    REQUIRE(loadedSameNoDefault == 0);
    REQUIRE(loadedDifferentNoDefault == 1);
    REQUIRE(loadedSameCustomDefault == 5);
    REQUIRE(loadedDifferentCustomDefault == 6);

    REQUIRE(noDefault.getDefaultValue() == 0);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    // Reset all values to their default values
    noDefault.resetToDefaultValue();
    customDefault.resetToDefaultValue();
    loadedSameNoDefault.resetToDefaultValue();
    loadedDifferentNoDefault.resetToDefaultValue();
    loadedSameCustomDefault.resetToDefaultValue();
    loadedDifferentCustomDefault.resetToDefaultValue();

    REQUIRE(noDefault.isDefaultValue());
    REQUIRE(customDefault.isDefaultValue());
    REQUIRE(loadedSameNoDefault.isDefaultValue());
    REQUIRE(loadedDifferentNoDefault.isDefaultValue());
    REQUIRE(loadedSameCustomDefault.isDefaultValue());
    REQUIRE(loadedDifferentCustomDefault.isDefaultValue());

    REQUIRE(noDefault.getDefaultValue() == 0);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);

    // After loading, after resetting all values to their default value
    REQUIRE(noDefault == 0);
    REQUIRE(customDefault == 5);
    REQUIRE(loadedSameNoDefault == 0);
    REQUIRE(loadedDifferentNoDefault == 0);
    REQUIRE(loadedSameCustomDefault == 5);
    REQUIRE(loadedDifferentCustomDefault == 5);

    noDefault.setDefaultValue(1);

    REQUIRE(noDefault == 0);

    REQUIRE(!noDefault.isDefaultValue());

    noDefault.resetToDefaultValue();

    REQUIRE(noDefault.isDefaultValue());

    REQUIRE(noDefault == 1);

    REQUIRE(noDefault.getDefaultValue() == 1);
    REQUIRE(customDefault.getDefaultValue() == 5);
    REQUIRE(loadedSameNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedDifferentNoDefault.getDefaultValue() == 0);
    REQUIRE(loadedSameCustomDefault.getDefaultValue() == 5);
    REQUIRE(loadedDifferentCustomDefault.getDefaultValue() == 5);
}
