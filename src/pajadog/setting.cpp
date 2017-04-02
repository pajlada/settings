#include "pajadog/setting.hpp"

#include <rapidjson/prettywriter.h>
#include <QFile>

#include <iostream>

namespace pajadog {
namespace settings {

rapidjson::Document *SettingsManager::document = nullptr;

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
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        std::cerr << "Unable to open file" << std::endl;
        return false;
    }

    document = new rapidjson::Document;

    auto fileData = file.readAll();

    document->Parse(fileData.constData());

    file.close();

    // This restricts config files a bit. They NEED to be
    if (!document->IsObject()) {
        std::cerr << "Error loading config file at " << path << std::endl;
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

    QFile file(QString::fromStdString(path));
    if (!file.open(QFile::WriteOnly)) {
        std::cerr << "Unable to open file" << std::endl;
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document->Accept(writer);

    file.write(buffer.GetString());

    file.close();

    return true;
}

}  // namespace settings
}  // namespace pajadog
