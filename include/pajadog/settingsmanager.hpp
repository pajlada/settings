#pragma once

#include <memory>
#include <vector>

namespace pajadog {

class ISettingData;

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

    static void registerSetting(std::shared_ptr<ISettingData> setting);

    // Force a settings save
    // It is recommended to run this every now and then unless your application
    // is crash free
    void save();

    std::vector<std::shared_ptr<ISettingData>> xDSettings;
};

}  // namespace pajadog
