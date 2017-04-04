#pragma once

#include <rapidjson/document.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace pajadog {
namespace settings {

template <typename Type>
class Setting;

class SettingsManager;

class ISettingData
{
public:
    ISettingData(const std::string &_key,
                 Setting<void> *_settingParent = nullptr)
        : key(_key)
        , settingParent(_settingParent)
        , connectionID(++this->latestConnectionID)
    {
    }

    inline const uint64_t
    getConnectionID() const
    {
        return this->connectionID;
    }

    virtual ~ISettingData() = default;

    rapidjson::Value *
    getJSONParent() const
    {
        return this->jsonParent;
    }

    rapidjson::Value *
    getJSONValue() const
    {
        return this->jsonValue;
    }

    Setting<void> *
    getSettingParent() const
    {
        return this->settingParent;
    }

    void
    setJSONParent(rapidjson::Value *newParent)
    {
        this->jsonParent = newParent;
    }

    const std::string &
    getKey() const
    {
        return this->key;
    }

    inline bool
    isFilled() const
    {
        return this->filled;
    }

    void
    setJSONValue(rapidjson::Value *newValue)
    {
        this->jsonValue = newValue;
    }

protected:
    // If the setting has been filled with any value other than the default one
    bool filled = false;

    rapidjson::Value *jsonParent = nullptr;
    rapidjson::Value *jsonValue = nullptr;
    Setting<void> *settingParent = nullptr;

    uint64_t connectionID = 0;
    static std::atomic<uint64_t> latestConnectionID;

private:
    // Setting key (i.e. "numThreads")
    const std::string key;
};

template <typename Type>
class SettingData : public ISettingData
{
public:
    SettingData(const std::string &_key, const Type &&defaultValue,
                Setting<void> *_settingParent)
        : ISettingData(_key, _settingParent)
        , data(defaultValue)
    {
    }

    SettingData(const std::string &_key, Setting<void> *_settingParent)
        : ISettingData(_key, _settingParent)
        , data(Type())
    {
    }

    void
    setValue(const Type &newValue)
    {
        this->data = newValue;

        this->filled = true;

        if (this->jsonValue != nullptr) {
            SettingsManager::JSONWrapper<Type>::setValue(this->jsonValue,
                                                         newValue);
        }
    }

    Type
    getValue() const
    {
        return this->data;
    }

    Type data;
};

template <>
class SettingData<void> : public ISettingData
{
public:
    SettingData(const std::string &_key, Setting<void> *_settingParent)
        : ISettingData(_key, _settingParent)
    {
    }
};

class ISetting
{
public:
    virtual ~ISetting() = default;

    Setting<void> *
    getParent()
    {
        return this->parent;
    }

protected:
    Setting<void> *parent = nullptr;

private:
    // Setting description (i.e. Number of threads to run the application in)
    std::string description;
};

template <typename Type>
class Setting : public ISetting
{
public:
    Setting(const std::string &key, const Type &&defaultValue,
            Setting<void> *_parent = nullptr)
        : data(new SettingData<Type>(key, std::move(defaultValue), _parent))
    {
        SettingsManager::registerSetting(this->data);
    }
    Setting(const std::string &key, Setting<void> *_parent = nullptr)
        : data(new SettingData<Type>(key, _parent))
    {
        SettingsManager::registerSetting(this->data);
    }

    ~Setting()
    {
        SettingsManager::unregisterSetting(this->data);
    }

    Setting &
    setName(const char *newName)
    {
        this->name = newName;

        return *this;
    }

    std::shared_ptr<SettingData<Type>>
    getData() const
    {
        return this->data;
    }

    const std::string &
    getKey() const
    {
        return this->data->getKey();
    }

    const std::string &
    getName() const
    {
        return this->name;
    }

    const Type
    getValue() const
    {
        assert(this->data != nullptr);

        return this->data->getValue();
    }

    const Type
    get() const
    {
        return this->getValue();
    }

    void
    setValue(const Type &newValue)
    {
        this->data->setValue(newValue);
    }

    Setting &
    operator=(const Type &newValue)
    {
        this->data->setValue(newValue);

        return *this;
    }

    Setting &
    operator=(Type &&newValue) noexcept
    {
        this->data->setValue(newValue);

        return *this;
    }

    operator const Type() const
    {
        return this->getValue();
    }

private:
    std::shared_ptr<SettingData<Type>> data;

    std::string name;
};

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

    template <typename JSONType>
    struct JSONWrapper {
        static void
        create(const std::shared_ptr<SettingData<void>> &)
        {
            static_assert(false, "Unimplemented JSONWrapper::create for Type");
        }
    };

    template <>
    struct JSONWrapper<void> {
        static rapidjson::Value
        create(const std::shared_ptr<SettingData<void>> &)
        {
            rapidjson::Value v;

            v.SetObject();

            return v;
        }
    };

    template <>
    struct JSONWrapper<int> {
        static rapidjson::Value
        create(const std::shared_ptr<SettingData<int>> &setting)
        {
            rapidjson::Value v;
            v.SetInt(setting->getValue());
            return v;
        }

        static void
        setValue(rapidjson::Value *jsonValue, const int &newValue)
        {
            jsonValue->SetInt(newValue);
        }
    };

    template <>
    struct JSONWrapper<bool> {
        static rapidjson::Value
        create(const std::shared_ptr<SettingData<bool>> &setting)
        {
            rapidjson::Value v;
            v.SetBool(setting->getValue());
            return v;
        }

        static void
        setValue(rapidjson::Value *jsonValue, const bool &newValue)
        {
            jsonValue->SetBool(newValue);
        }
    };

    template <>
    struct JSONWrapper<float> {
        static rapidjson::Value
        create(const std::shared_ptr<SettingData<float>> &setting)
        {
            rapidjson::Value v;
            v.SetFloat(setting->getValue());
            return v;
        }

        static void
        setValue(rapidjson::Value *jsonValue, const float &newValue)
        {
            jsonValue->SetFloat(newValue);
        }
    };

    template <>
    struct JSONWrapper<double> {
        static rapidjson::Value
        create(const std::shared_ptr<SettingData<double>> &setting)
        {
            rapidjson::Value v;
            v.SetDouble(setting->getValue());
            return v;
        }

        static void
        setValue(rapidjson::Value *jsonValue, const double &newValue)
        {
            jsonValue->SetDouble(newValue);
        }
    };

    template <>
    struct JSONWrapper<std::string> {
        static rapidjson::Value
        create(const std::shared_ptr<SettingData<std::string>> &setting)
        {
            rapidjson::Value v;
            v.SetString(rapidjson::StringRef(setting->getValue().c_str()));
            return v;
        }

        static void
        setValue(rapidjson::Value *jsonValue, const std::string &newValue)
        {
            jsonValue->SetString(newValue.c_str(),
                                 SettingsManager::document->GetAllocator());
        }
    };

    std::string path = "settings.json";

    template <typename Type>
    static void
    unregisterSetting(const std::shared_ptr<SettingData<Type>> &setting)
    {
        SettingsManager::localUnregister(setting);
    }

    template <typename Type>
    static void
    registerSetting(std::shared_ptr<SettingData<Type>> setting)
    {
        requireManager();

        if (loaded) {
            // If settings are already loaded from a file, try to fill in the
            // settings
            manager->loadSetting(setting);
        }

        SettingsManager::localRegister(std::move(setting));
    }

    static SettingsManager *manager;

    static bool loaded;

    static void setPath(const char *path);

    // Clear the loaded json settings
    static void clear();

    // Load from given path and set given path as the "default path" (or load
    // from default path if nullptr is sent)
    static bool load(const char *path = nullptr);
    // Load from given path
    static bool loadFrom(const char *path);

    // Force a settings save
    // It is recommended to run this every now and then unless your application
    // is crash free
    // Save to given path and set path as the default path (or save from default
    // path is nullptr is sent
    static bool save(const char *path = nullptr);
    // Save to given path
    static bool saveAs(const char *path);

    std::vector<std::shared_ptr<SettingData<int>>> intSettings;
    std::vector<std::shared_ptr<SettingData<bool>>> boolSettings;
    std::vector<std::shared_ptr<SettingData<std::string>>> strSettings;
    std::vector<std::shared_ptr<SettingData<double>>> doubleSettings;
    std::vector<std::shared_ptr<SettingData<float>>> floatSettings;
    std::vector<std::shared_ptr<SettingData<void>>> objectSettings;

    static rapidjson::Document *document;

    template <typename Type>
    static rapidjson::Value *
    getSettingParent(std::shared_ptr<SettingData<Type>> &setting)
    {
        auto settingParent = setting->getSettingParent();
        if (settingParent == nullptr) {
            return SettingsManager::document;
        }

        // Has the setting parent been loaded/created yet? (does it have a
        // jsonValue yet?)
        auto parentData = settingParent->getData();

        if (parentData->getJSONValue() != nullptr) {
            return parentData->getJSONValue();
        }

        // returning nullptr means that we should give this another pass
        return nullptr;
    }

    template <typename Type>
    bool
    loadSetting(std::shared_ptr<SettingData<Type>> setting)
    {
        // Sanity check
        assert(loaded == true);

        // TODO: fix parentage

        rapidjson::Value *parent = getSettingParent(setting);

        setting->setJSONParent(parent);

        // Find the key at root level
        if (!parent->IsObject() && !parent->IsArray()) {
            // Parent must be either an object or an array
            std::cerr << "Parent must be either an object or an array"
                      << std::endl;
            return false;
        }

        const char *settingKey = setting->getKey().c_str();

        // XXX(pajlada): For now we assume that parents are always objects
        // TODO(pajlada): Implement support for parent arrays
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

        return false;
    }

    template <typename Type>
    bool
    setSetting(std::shared_ptr<SettingData<Type>> setting,
               const rapidjson::Value &value)
    {
        static_assert(false, "Unimplemented setSetting for type");
        return false;
    }

    template <>
    bool
    setSetting<void>(std::shared_ptr<SettingData<void>>,
                     const rapidjson::Value &)
    {
        // Do nothing
        // Void = object type
        return true;
    }

    template <>
    bool
    setSetting<float>(std::shared_ptr<SettingData<float>> setting,
                      const rapidjson::Value &value)
    {
        auto type = value.GetType();

        switch (type) {
            case rapidjson::Type::kNumberType: {
                if (value.IsDouble()) {
                    setting->setValue(static_cast<float>(value.GetDouble()));
                    return true;
                } else if (value.IsFloat()) {
                    setting->setValue(value.GetFloat());
                    return true;
                } else if (value.IsInt()) {
                    setting->setValue(static_cast<float>(value.GetInt()));
                    return true;
                }
            } break;
        }

        return false;
    }

    template <>
    bool
    setSetting<double>(std::shared_ptr<SettingData<double>> setting,
                       const rapidjson::Value &value)
    {
        auto type = value.GetType();

        switch (type) {
            case rapidjson::Type::kNumberType: {
                if (value.IsDouble()) {
                    setting->setValue(value.GetDouble());
                    return true;
                } else if (value.IsInt()) {
                    setting->setValue(value.GetInt());
                    return true;
                }
            } break;
        }

        return false;
    }

    template <>
    bool
    setSetting<std::string>(std::shared_ptr<SettingData<std::string>> setting,
                            const rapidjson::Value &value)
    {
        auto type = value.GetType();

        switch (type) {
            case rapidjson::Type::kStringType: {
                setting->setValue(value.GetString());
                return true;
            } break;
        }

        return false;
    }

    template <>
    bool
    setSetting<bool>(std::shared_ptr<SettingData<bool>> setting,
                     const rapidjson::Value &value)
    {
        auto type = value.GetType();

        switch (type) {
            case rapidjson::Type::kTrueType:
            case rapidjson::Type::kFalseType: {
                setting->setValue(value.GetBool());
                return true;
            } break;

            case rapidjson::Type::kNumberType: {
                if (value.IsInt()) {
                    setting->setValue(value.GetInt() == 1);
                    return true;
                }
            } break;
        }

        return false;
    }

    template <>
    bool
    setSetting<int>(std::shared_ptr<SettingData<int>> setting,
                    const rapidjson::Value &value)
    {
        auto type = value.GetType();

        switch (type) {
            case rapidjson::Type::kNumberType: {
                if (value.IsDouble()) {
                    setting->setValue(static_cast<int>(value.GetDouble()));
                    return true;
                } else if (value.IsInt()) {
                    setting->setValue(value.GetInt());
                    return true;
                }
            } break;
        }

        return false;
    }

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
    checkSaveMethodFlag(SettingsManager::SaveMethod testSaveMethod) const
    {
        return (static_cast<uint64_t>(this->saveMethod) &
                static_cast<uint64_t>(testSaveMethod)) != 0;
    }

    static void
    requireManager()
    {
        if (manager == nullptr) {
            manager = new SettingsManager;
        }
    }

private:
    template <typename Type>
    static void
    localRegister(std::shared_ptr<SettingData<Type>> setting)
    {
        static_assert(false, "Unimplemented localRegister for setting type");
    }

    template <typename Type>
    static void
    localUnregister(const std::shared_ptr<SettingData<Type>> &setting)
    {
        static_assert(false, "Unimplemented localUnregister for setting type");
    }

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

    template <>
    static void
    localRegister<void>(std::shared_ptr<SettingData<void>> setting)
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

    template <>
    static void
    localUnregister<void>(const std::shared_ptr<SettingData<void>> &setting)
    {
        SettingsManager::removeSettingFrom(manager->objectSettings, setting);
    }

    template <>
    static void
    localUnregister<bool>(const std::shared_ptr<SettingData<bool>> &setting)
    {
        SettingsManager::removeSettingFrom(manager->boolSettings, setting);
    }

    template <>
    static void
    localUnregister<int>(const std::shared_ptr<SettingData<int>> &setting)
    {
        SettingsManager::removeSettingFrom(manager->intSettings, setting);
    }

    template <>
    static void
    localUnregister<std::string>(
        const std::shared_ptr<SettingData<std::string>> &setting)
    {
        SettingsManager::removeSettingFrom(manager->strSettings, setting);
    }

    template <>
    static void
    localUnregister<float>(const std::shared_ptr<SettingData<float>> &setting)
    {
        SettingsManager::removeSettingFrom(manager->floatSettings, setting);
    }

    template <>
    static void
    localUnregister<double>(const std::shared_ptr<SettingData<double>> &setting)
    {
        SettingsManager::removeSettingFrom(manager->doubleSettings, setting);
    }
};

template <>
class Setting<void> : public ISetting
{
public:
    Setting(const std::string &key, Setting<void> *_settingParent = nullptr)
        : data(new SettingData<void>(key, _settingParent))
    {
        SettingsManager::registerSetting(this->data);
    }

    Setting &
    setName(const char *newName)
    {
        this->name = newName;

        return *this;
    }

    std::shared_ptr<SettingData<void>>
    getData() const
    {
        return this->data;
    }

    const std::string &
    getKey() const
    {
        return this->data->getKey();
    }

    const std::string &
    getName() const
    {
        return this->name;
    }

private:
    std::shared_ptr<SettingData<void>> data;

    std::string name;
};

}  // namespace setting
}  // namespace pajadog
