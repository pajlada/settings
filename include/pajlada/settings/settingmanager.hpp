#pragma once

#include <pajlada/settings/common.hpp>
#include <pajlada/settings/signalargs.hpp>

#include <rapidjson/pointer.h>

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace pajlada {
namespace Settings {

class SettingData;

class SettingManager
{
public:
    SettingManager();
    ~SettingManager();

    enum class LoadError {
        NoError,
        CannotOpenFile,
        FileHandleError,
        FileReadError,
        FileSeekError,
        JSONParseError,
    };

    // Print given document json data prettily
    void pp(const std::string &prefix = std::string());
    static void gPP(const std::string &prefix = std::string());
    static std::string stringify(const rapidjson::Value &v);

    rapidjson::Value *get(const char *path);
    bool set(const char *path, const rapidjson::Value &value,
             SignalArgs args = SignalArgs());

private:
    // Called from set
    void notifyUpdate(const std::string &path, const rapidjson::Value &value,
                      SignalArgs args = SignalArgs());

    // Called from load
    void notifyLoadedValues();

public:
    // Useful array helper methods
    static rapidjson::SizeType arraySize(const std::string &path);
    static bool isNull(const std::string &path);
    bool _isNull(const std::string &path);
    static void setNull(const std::string &path);

    // Basically the same as setNull, except we fully remove a value if it's the
    // last index of the array
    static bool removeArrayValue(const std::string &arrayPath,
                                 rapidjson::SizeType index);

    static rapidjson::SizeType cleanArray(const std::string &arrayPath);

    // Useful object helper methods
    static std::vector<std::string> getObjectKeys(
        const std::string &objectPath);

    static void clear();

    static std::weak_ptr<SettingData> getSetting(
        const std::string &path, std::shared_ptr<SettingManager> instance);

    static bool removeSetting(const std::string &path);

private:
    template <typename Type>
    friend class Setting;

    bool _removeSetting(const std::string &path);

    void clearSettings(const std::string &root);

public:
    void setPath(const char *newFilePath);

    // Load from given path and set given path as the "default path" (or load
    // from default path if nullptr is sent)
    LoadError load(const char *filePath = nullptr);
    // Load from given path
    LoadError loadFrom(const char *filePath);

    // Force a settings save
    // It is recommended to run this every now and then unless your application
    // is crash free
    // Save to given path and set path as the default path (or save from default
    // path if filePath is a nullptr)
    bool save(const std::string &path = std::string());
    // Save to given path
    bool saveAs(const std::string &path);

private:
    bool _save(const std::string &path);

public:
    // Functions prefixed with g are static functions that work
    // on the statically initialized SettingManager instance
    static LoadError gLoad(const char *filePath = nullptr);
    static LoadError gLoadFrom(const char *filePath);
    static bool gSave(const char *filePath = nullptr);
    static bool gSaveAs(const char *filePath);

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

    struct {
        bool enabled{};
        uint8_t slots = 3;
    } backup;

public:
    void setBackupEnabled(bool enabled = true);
    void setBackupSlots(uint8_t numSlots);

    static const std::shared_ptr<SettingManager> &
    getInstance()
    {
        static auto m = std::make_shared<SettingManager>();

        return m;
    }

private:
    std::shared_ptr<SettingData> getSetting(const std::string &path);

public:
    rapidjson::Document document;

private:
    std::string filePath = "settings.json";

    std::mutex settingsMutex;

    //       path         setting
    std::map<std::string, std::shared_ptr<SettingData>> settings;
};

}  // namespace Settings
}  // namespace pajlada
