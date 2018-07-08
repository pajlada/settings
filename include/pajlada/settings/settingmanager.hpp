#pragma once

#include "pajlada/settings/settingdata.hpp"

#include <rapidjson/pointer.h>

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>

namespace pajlada {
namespace Settings {

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
    static void pp(const std::string &prefix = std::string());
    static void ppDocument(const rapidjson::Document &document,
                           const std::string &prefix = std::string());
    static std::string stringify(const rapidjson::Value &v);

    static rapidjson::Value *rawValue(const char *path);
    static rapidjson::Value *get(const char *path, rapidjson::Document &d);
    static void set(const char *path, rapidjson::Value &&value,
                    rapidjson::Document &d);
    static void set(const char *path, rapidjson::Value &&value);

    static void setWithPointer(const char *path, rapidjson::Value &&value);

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

    template <typename Type, typename Container>
    static std::weak_ptr<Container>
    getSetting(const std::string &path, SettingOption options,
               std::shared_ptr<SettingManager> instance)
    {
        const auto creator = [](auto &setting) {
            setting.reset(new Container);  //
        };

        return SettingManager::_getSetting<Type, Container>(path, options,
                                                            creator, instance);
    }

    template <typename Type, typename Container>
    static std::weak_ptr<Container>
    getSetting(const std::string &path, const Type &defaultValue,
               SettingOption options, std::shared_ptr<SettingManager> instance)
    {
        const auto creator = [&defaultValue](auto &setting) {
            setting.reset(new Container(defaultValue));  //
        };

        return SettingManager::_getSetting<Type, Container>(path, options,
                                                            creator, instance);
    }

    template <typename Type, typename Container>
    static std::weak_ptr<Container>
    getSetting(const std::string &path, const Type &defaultValue,
               const Type &currentValue, SettingOption options,
               std::shared_ptr<SettingManager> instance)
    {
        const auto creator = [&defaultValue, &currentValue](
                                 std::shared_ptr<Container> &setting) {
            setting.reset(new Container(defaultValue, currentValue));  //
        };

        return SettingManager::_getSetting<Type, Container>(path, options,
                                                            creator, instance);
    }

    static bool removeSetting(const std::string &path);

private:
    template <typename Type>
    friend class Setting;

    void registerSetting(std::shared_ptr<ISettingData> &setting);

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
    bool save(const char *filePath = nullptr);
    // Save to given path
    bool saveAs(const char *filePath);

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

    static const std::shared_ptr<SettingManager> &
    getInstance()
    {
        static auto m = std::make_shared<SettingManager>();

        return m;
    }

    template <typename Type, typename Container, typename Functor>
    static std::weak_ptr<Container>
    _getSetting(const std::string &path, SettingOption options,
                Functor creatorFunc, std::shared_ptr<SettingManager> instance)
    {
        if (!instance) {
            instance = SettingManager::getInstance();
        }

        std::lock_guard<std::mutex> lock(instance->settingsMutex);

        auto &setting = instance->settings[path];

        if (setting == nullptr) {
            // No setting has been created with this path
            creatorFunc(setting);

            // TODO: This should be in the constructor
            setting->setPath(path);

            setting->options = options;

            if (!setting->optionEnabled(SettingOption::Remote)) {
                instance->registerSetting(setting);
            }
        } else if (setting->optionEnabled(SettingOption::ForceSetOptions)) {
            setting->options = options;
        }

        return std::static_pointer_cast<Container>(setting);
    }

    rapidjson::Document document;

    std::string filePath = "settings.json";

    std::mutex settingsMutex;

    //       path         setting
    std::map<std::string, std::shared_ptr<ISettingData>> settings;
};

}  // namespace Settings
}  // namespace pajlada
