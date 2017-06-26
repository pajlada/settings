#pragma once

#include "pajlada/settings/types.hpp"

#include <rapidjson/pointer.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

namespace pajlada {
namespace Settings {

class ISettingData;

class SettingManager
{
    SettingManager();
    ~SettingManager();

public:
    enum class LoadError {
        NoError,
        CannotOpenFile,
        FileHandleError,
        FileReadError,
        FileSeekError,
        JSONParseError,
    };

    // Print given document json data prettily
    static void pp();
    static void ppDocument(const rapidjson::Document &document);

private:
    template <typename Type, typename Container>
    friend class Setting;

    static void unregisterSetting(const std::shared_ptr<ISettingData> &setting);

    static void registerSetting(std::shared_ptr<ISettingData> setting);

public:
    // Load from given path and set given path as the "default path" (or load
    // from default path if nullptr is sent)
    static LoadError load(const char *filePath = nullptr);
    // Load from given path
    static LoadError loadFrom(const char *filePath);

    // Force a settings save
    // It is recommended to run this every now and then unless your application
    // is crash free
    // Save to given path and set path as the default path (or save from default
    // path if filePath is a nullptr)
    static bool save(const char *filePath = nullptr);
    // Save to given path
    static bool saveAs(const char *filePath);

    enum class SaveMethod : uint64_t {
        SaveOnExitFlag = (1ull << 1ull),
        SaveOnSettingChangeFlag = (1ull << 2ull),
        SaveInitialValue = (1ull << 3ull),

        // Force user to manually call SettingsManager::save() to save
        SaveManually = 0,
        SaveAllTheTime = SaveOnExitFlag | SaveOnSettingChangeFlag,
    } saveMethod = SaveMethod::SaveAllTheTime;

private:
    // Returns true if the given save method is activated
    inline bool
    checkSaveMethodFlag(SettingManager::SaveMethod testSaveMethod) const
    {
        return (static_cast<uint64_t>(this->saveMethod) &
                static_cast<uint64_t>(testSaveMethod)) != 0;
    }

    static SettingManager &
    getInstance()
    {
        static SettingManager m;

        return m;
    }

    rapidjson::Document document;

    std::string filePath = "settings.json";

    std::mutex settingsVectorMutex;
    std::vector<std::shared_ptr<ISettingData>> settings;
};

}  // namespace Settings
}  // namespace pajlada
