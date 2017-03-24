#include "pajadog/setting.hpp"
#include "pajadog/settingsmanager.hpp"

#include <iostream>

namespace pajadog {

Setting::Setting(ISettingData *_data)
    : data(_data)
{
    SettingsManager::registerSetting(this->data);

    /*
    std::cout << "ctor Setting: " << this->data->getKey().toStdString()
              << std::endl;
    */
}

Setting::~Setting()
{
    /*
    std::cout << "dtor Setting: " << this->data->getKey().toStdString()
              << std::endl;
              */
}

}  // namespace pajadog
