#include "pajadog/settingsmanager.hpp"
#include "pajadog/setting.hpp"

#include <iostream>
#include <vector>

namespace pajadog {

static SettingsManager manager;
// static std::vector<std::shared_ptr<Setting>> settings;

SettingsManager::SettingsManager()
{
    std::cout << "ctor SettingsManager" << std::endl;
}

SettingsManager::~SettingsManager()
{
    this->save();

    std::cout << "dtor SettingsManager" << std::endl;

    this->xDSettings.clear();
}

void
SettingsManager::registerSetting(std::shared_ptr<ISettingData> setting)
{
    /*
    std::cout << "Registering setting " << setting->getKey().toStdString()
              << std::endl;
    manager.xDSettings.push_back(std::move(setting));
              */
}

void
SettingsManager::save()
{
    std::cout << "Saving settings!" << std::endl;
}

}  // namespace pajadog
