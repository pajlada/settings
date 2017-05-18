#include "pajlada/settings/setting.hpp"

#include <assert.h>

class Test
{
public:
    Test(int index)
        : rootSetting("/a/b/c_" + std::to_string(index))
        , name("name", this->rootSetting)
        , alias("alias", this->rootSetting)
        , leafSetting("f", this->rootSetting)
    {
    }

    pajlada::Settings::Setting<pajlada::Settings::Object> rootSetting;
    pajlada::Settings::Setting<std::string> name;
    pajlada::Settings::Setting<std::string> alias;
    pajlada::Settings::Setting<pajlada::Settings::Object> leafSetting;
};

int
main(int argc, char **argv)
{
    Test t1(1);

    assert(t1.name.getValue() == "");

    pajlada::Settings::SettingManager::load("settings.json");

    assert(t1.name.getValue() == "LUL");

    Test t2(2);

    assert(t2.name.getValue() == "ZULUL");

    pajlada::Settings::SettingManager::save();

    return 0;
}
