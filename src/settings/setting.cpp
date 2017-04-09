#include "pajlada/settings/setting.hpp"
#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"

#include <rapidjson/prettywriter.h>

#include <iostream>
#include <vector>

namespace pajlada {
namespace settings {

rapidjson::Document *SettingManager::document = nullptr;
std::atomic<uint64_t> ISettingData::latestConnectionID = 0;

SettingManager *SettingManager::manager = nullptr;
bool SettingManager::loaded = false;

}  // namespace settings
}  // namespace pajlada
