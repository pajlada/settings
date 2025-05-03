#pragma once

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

#include <algorithm>
#include <atomic>
#include <cinttypes>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <pajlada/settings/backup.hpp>
#include <pajlada/settings/common.hpp>
#include <pajlada/settings/signalargs.hpp>
#include <shared_mutex>
#include <vector>

namespace pajlada::Settings {

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

    enum class SaveResult : std::uint8_t {
        /// Saving the settings to a file failed
        /// We currently don't elaborate why it failed
        Failed,

        /// The settings were successfully saved to a file
        Success,

        /// No save was attempted because we deemed it unneccessary
        Skipped,
    };

    // Print given document json data prettily
    void pp(const std::string &prefix = std::string());
    static void gPP(const std::string &prefix = std::string());
    static std::string stringify(const rapidjson::Value &v);

    bool get(const std::string &path, rapidjson::Document &doc);

    decltype(auto)
    get(const std::string &path, auto &&fn)
    {
        std::shared_lock g(this->settingsDataMutex);
        auto *ptr = this->rawGet(path);
        return fn(ptr);
    }

    bool compare(const std::string &path, const rapidjson::Value &value);

    bool set(const std::string &path, const rapidjson::Value &value,
             const SignalArgs &args = SignalArgs());

private:
    // Called from set
    void notifyUpdate(const std::string &path, const rapidjson::Value &value,
                      const SignalArgs &args = {});

    // Called from load
    void notifyLoadedValues();

    rapidjson::Value *rawGet(const std::string &path);

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
    void setPath(const std::filesystem::path &newPath);

    static LoadError gLoad(const std::filesystem::path &path = {});
    static LoadError gLoadFrom(const std::filesystem::path &path);

    // Load from given path and set given path as the "default path" (or load
    // from default path if nullptr is sent)
    LoadError load(const std::filesystem::path &path = {});
    // Load from given path
    LoadError loadFrom(const std::filesystem::path &path);

    static SaveResult gSave(const std::filesystem::path &path = {});
    static SaveResult gSaveAs(const std::filesystem::path &path);

    // Force a settings save
    // It is recommended to run this every now and then unless your application
    // is crash free
    // Save to given path and set path as the default path (or save from default
    // path if filePath is a nullptr)
    SaveResult save(const std::filesystem::path &path = {});
    // Save to given path
    SaveResult saveAs(const std::filesystem::path &path);

private:
    bool writeTo(const std::filesystem::path &path);

public:
    // Functions prefixed with g are static functions that work
    // on the statically initialized SettingManager instance

    enum class SaveMethod : uint64_t {
        SaveOnExit = (1ULL << 1ULL),
        SaveOnSettingChange = (1ULL << 2ULL),

        /// Only perform the save (& and backup) if a call to `set` has come in, meaning
        /// some setting has changed.
        ///
        /// Pairs well with `SettingOption::CompareBeforeSet`, ensuring `set` does not
        /// set the `hasUnsavedChanges` flag unnecessarily.
        OnlySaveIfChanged = (1ULL << 3ULL),

        /// Force user to manually call SettingsManager::save() to save
        SaveManually = 0,
        SaveAllTheTime = SaveOnExit | SaveOnSettingChange,
    } saveMethod = SaveMethod::SaveOnExit;

private:
    /// Set to true by `set` if a value has changed
    /// Reset to false when a save has succeeded
    std::atomic<bool> hasUnsavedChanges = false;

    // Returns true if the given save method is activated
    inline bool
    hasSaveMethodFlag(SettingManager::SaveMethod testSaveMethod) const
    {
        return (static_cast<uint64_t>(this->saveMethod) &
                static_cast<uint64_t>(testSaveMethod)) != 0;
    }

    Backup::Options backup;

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
    std::filesystem::path filePath = "settings.json";

    std::mutex settingsMutex;

    std::shared_mutex settingsDataMutex;

    //       path         setting
    std::map<std::string, std::shared_ptr<SettingData>> settings;
};

}  // namespace pajlada::Settings
