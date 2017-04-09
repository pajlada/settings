#pragma once

#include "pajlada/settings/jsonwrapper.hpp"

#include <vector>

namespace pajlada {
namespace settings {

template <typename Type>
class SettingData;

class SettingManager
{
public:
    SettingManager();
    ~SettingManager();

    std::string path = "settings.json";

    template <typename Type>
    static void
    unregisterSetting(const std::shared_ptr<SettingData<Type>> &setting)
    {
        SettingManager::localUnregister(setting);
    }

    template <typename Type>
    static void
    registerSetting(std::shared_ptr<SettingData<Type>> setting)
    {
        requireManager();

        if (loaded) {
            // If settings are already loaded from a file, try to fill in
            // the
            // settings
            manager->loadSetting(setting);
        }

        SettingManager::localRegister(std::move(setting));
    }

    static SettingManager *manager;

    static bool loaded;

    static void setPath(const char *path);

    // Clear the loaded json settings
    static void clear();

    // Load from given path and set given path as the "default path" (or
    // load
    // from default path if nullptr is sent)
    static bool load(const char *path = nullptr);
    // Load from given path
    static bool loadFrom(const char *path);

    // Force a settings save
    // It is recommended to run this every now and then unless your
    // application
    // is crash free
    // Save to given path and set path as the default path (or save from
    // default
    // path is nullptr is sent
    static bool save(const char *path = nullptr);
    // Save to given path
    static bool saveAs(const char *path);

    std::vector<std::shared_ptr<SettingData<int>>> intSettings;
    std::vector<std::shared_ptr<SettingData<bool>>> boolSettings;
    std::vector<std::shared_ptr<SettingData<std::string>>> strSettings;
    std::vector<std::shared_ptr<SettingData<double>>> doubleSettings;
    std::vector<std::shared_ptr<SettingData<float>>> floatSettings;
    std::vector<std::shared_ptr<SettingData<Object>>> objectSettings;
    std::vector<std::shared_ptr<SettingData<Array>>> arraySettings;

    static rapidjson::Document *document;

    template <typename Type>
    static rapidjson::Value *getSettingParent(
        std::shared_ptr<SettingData<Type>> &setting);

    template <typename Type>
    bool
    loadSetting(std::shared_ptr<SettingData<Type>> setting)
    {
        // Sanity check
        assert(loaded == true);

        rapidjson::Value *parent = getSettingParent(setting);

        if (parent == nullptr) {
            // Parent not loaded yet, re-do in second pass
            return false;
        }

        setting->setJSONParent(parent);

        if (parent->IsObject()) {
            return this->loadSettingFromObject(setting, parent);
        } else if (parent->IsArray()) {
            return this->loadSettingFromArray(setting, parent);
        }

        // Parent must be either an object or an array
        std::cerr << "Parent must be either an object or an array" << std::endl;
        return false;
    }

    template <typename Type>
    bool
    loadSettingFromObject(std::shared_ptr<SettingData<Type>> setting,
                          rapidjson::Value *parent)
    {
        const char *settingKey = setting->getKey().c_str();

        if (parent->HasMember(settingKey)) {
            const rapidjson::Value &settingValue = (*parent)[settingKey];

            setting->setJSONValue(
                &const_cast<rapidjson::Value &>(settingValue));

            this->setSetting(setting, settingValue);
        } else {
            rapidjson::Value key(settingKey, document->GetAllocator());
            rapidjson::Value createdValue = JSONWrapper<Type>::create(setting);

            parent->AddMember(key, createdValue, document->GetAllocator());

            setting->setJSONValue(&(*parent)[settingKey]);
        }

        return true;
    }

    template <typename Type>
    bool
    loadSettingFromArray(std::shared_ptr<SettingData<Type>> setting,
                         rapidjson::Value *parent)
    {
        const unsigned index = setting->getIndex();

        if (index < parent->Size()) {
            rapidjson::Value &settingValue = (*parent)[index];

            setting->setJSONValue(&settingValue);

            this->setSetting(setting, settingValue);
        } else if (index == parent->Size()) {
            // Just out of reach, create new object
            rapidjson::Value createdValue = JSONWrapper<Type>::create(setting);
            // rapidjson::Value createdValue("xd", document->GetAllocator());

            parent->PushBack(createdValue.Move(), document->GetAllocator());

            std::cout << "New capacity: " << parent->Size() << std::endl;

            setting->setJSONValue(&(*parent)[index]);
        }

        return true;
    }

    template <typename Type>
    bool setSetting(std::shared_ptr<SettingData<Type>> setting,
                    const rapidjson::Value &value);

    enum class SaveMethod : uint64_t {
        SaveOnExitFlag = (1ull << 1ull),
        SaveOnSettingChangeFlag = (1ull << 2ull),

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

    static void
    requireManager()
    {
        if (manager == nullptr) {
            manager = new SettingManager;
        }
    }

private:
    template <class Vector, typename Type>
    static void
    removeSettingFrom(Vector &vec,
                      const std::shared_ptr<SettingData<Type>> &setting)
    {
        vec.erase(std::remove_if(std::begin(vec), std::end(vec),
                                 [setting](const auto &item) {
                                     return setting->getConnectionID() ==
                                            item->getConnectionID();
                                 }),
                  std::end(vec));
    }

    template <typename Type>
    static void
    localRegister(std::shared_ptr<SettingData<Type>> setting)
    {
        static_assert(false, "Unimplemented localRegister for setting type");
    }

    template <>
    static void
    localRegister<Array>(std::shared_ptr<SettingData<Array>> setting)
    {
        manager->arraySettings.push_back(setting);
    }

    template <>
    static void
    localRegister<Object>(std::shared_ptr<SettingData<Object>> setting)
    {
        manager->objectSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<bool>(std::shared_ptr<SettingData<bool>> setting)
    {
        manager->boolSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<int>(std::shared_ptr<SettingData<int>> setting)
    {
        manager->intSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<std::string>(
        std::shared_ptr<SettingData<std::string>> setting)
    {
        manager->strSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<float>(std::shared_ptr<SettingData<float>> setting)
    {
        manager->floatSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<double>(std::shared_ptr<SettingData<double>> setting)
    {
        manager->doubleSettings.push_back(setting);
    }

    template <typename Type>
    static void
    localUnregister(const std::shared_ptr<SettingData<Type>> &setting)
    {
        static_assert(false, "Unimplemented localUnregister for setting type");
        static bool const value = Type::value;
    }

    template <>
    static void
    localUnregister<Array>(const std::shared_ptr<SettingData<Array>> &setting)
    {
        SettingManager::removeSettingFrom(manager->arraySettings, setting);
    }

    template <>
    static void
    localUnregister<Object>(const std::shared_ptr<SettingData<Object>> &setting)
    {
        SettingManager::removeSettingFrom(manager->objectSettings, setting);
    }

    template <>
    static void
    localUnregister<bool>(const std::shared_ptr<SettingData<bool>> &setting)
    {
        SettingManager::removeSettingFrom(manager->boolSettings, setting);
    }

    template <>
    static void
    localUnregister<int>(const std::shared_ptr<SettingData<int>> &setting)
    {
        SettingManager::removeSettingFrom(manager->intSettings, setting);
    }

    template <>
    static void
    localUnregister<std::string>(
        const std::shared_ptr<SettingData<std::string>> &setting)
    {
        SettingManager::removeSettingFrom(manager->strSettings, setting);
    }

    template <>
    static void
    localUnregister<float>(const std::shared_ptr<SettingData<float>> &setting)
    {
        SettingManager::removeSettingFrom(manager->floatSettings, setting);
    }

    template <>
    static void
    localUnregister<double>(const std::shared_ptr<SettingData<double>> &setting)
    {
        SettingManager::removeSettingFrom(manager->doubleSettings, setting);
    }
};

}  // namespace setting
}  // namespace pajlada
