#include "pajadog/setting.hpp"

#include <rapidjson/prettywriter.h>

#include <fstream>
#include <iostream>
#include <vector>

namespace pajadog {
namespace settings {

rapidjson::Document *SettingsManager::document = nullptr;
std::atomic<uint64_t> ISettingData::latestConnectionID = 0;

SettingsManager *SettingsManager::manager = nullptr;
bool SettingsManager::loaded = false;

SettingsManager::SettingsManager()
{
    // XXX(pajlada): Load "default.json"?
}

SettingsManager::~SettingsManager()
{
    // XXX(pajlada): Should settings automatically save on exit?
    // Or on each setting change?
    // Or only manually?
    if (this->checkSaveMethodFlag(SaveMethod::SaveOnExitFlag)) {
        SettingsManager::save();
    }
}

void
SettingsManager::setPath(const char *path)
{
    manager->path = path;
}

void
SettingsManager::clear()
{
    document = nullptr;
    loaded = false;
}

bool
SettingsManager::load(const char *path)
{
    if (path != nullptr) {
        manager->path = path;
    }

    return SettingsManager::loadFrom(manager->path.c_str());
}

bool
SettingsManager::loadFrom(const char *path)
{
    // Open file
    std::ifstream fs(path, std::ios::binary | std::ios::ate);
    if (!fs) {
        return false;
    }

    // Read size of file
    std::streamsize fileSize = fs.tellg();
    fs.seekg(0, std::ios::beg);

    // Create vector of appropriate size
    std::vector<char> fileBuffer(fileSize);

    // Read file data into vector
    fs.read(fileBuffer.data(), fileSize);
    if (!fs) {
        return false;
    }

    document = new rapidjson::Document;
    document->Parse(fileBuffer.data(), fileSize);

    // Close file
    fs.close();

    // This restricts config files a bit. They NEED to have an object root
    if (!document->IsObject()) {
        return false;
    }

    loaded = true;

    // Fill in any settings that registered before we called load
    for (auto &setting : manager->objectSettings) {
        if (manager->loadSetting(setting)) {
        }
    }
    for (auto &setting : manager->intSettings) {
        if (manager->loadSetting(setting)) {
        }
    }
    for (auto &setting : manager->boolSettings) {
        if (manager->loadSetting(setting)) {
        }
    }
    for (auto &setting : manager->strSettings) {
        if (manager->loadSetting(setting)) {
        }
    }
    for (auto &setting : manager->doubleSettings) {
        if (manager->loadSetting(setting)) {
        }
    }
    for (auto &setting : manager->floatSettings) {
        if (manager->loadSetting(setting)) {
        }
    }

    return true;
}

bool
SettingsManager::save(const char *path)
{
    if (path != nullptr) {
        manager->path = path;
    }

    return SettingsManager::saveAs(manager->path.c_str());
}

bool
SettingsManager::saveAs(const char *path)
{
    if (!loaded) {
        return false;
    }

    std::ofstream fs(path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!fs) {
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document->Accept(writer);

    fs.write(buffer.GetString(), buffer.GetSize());
    if (!fs) {
        return false;
    }

    fs.close();

    return true;
}

}  // namespace settings
}  // namespace pajadog
