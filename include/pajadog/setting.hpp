#pragma once

#include <rapidjson/document.h>
#include <QString>

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace pajadog {
namespace settings {

class SettingsManager;

template <typename Type>
class SettingData
{
public:
    SettingData(const QString &_key, const Type &&defaultValue)
        : key(_key)
        , data(defaultValue)
    {
    }

    rapidjson::Value *
    getParent() const
    {
        return this->jsonParent;
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

    const QString &
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
    setJSONParent(rapidjson::Value *newParent)
    {
        this->jsonParent = newParent;
    }

    void
    setJSONValue(rapidjson::Value *newValue)
    {
        this->jsonValue = newValue;
    }

private:
    // Setting key (i.e. "numThreads")
    const QString key;

    // Setting description (i.e. Number of threads to run the application in)
    QString description;

    // If the setting has been filled with any value other than the default one
    bool filled = false;

    rapidjson::Value *jsonParent = nullptr;
    rapidjson::Value *jsonValue = nullptr;
};

template <>
class SettingData<void>
{
public:
    SettingData(const QString &_key)
        : key(_key)
    {
    }

    rapidjson::Value *
    getParent() const
    {
        return this->jsonParent;
    }

    const QString &
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
    setJSONParent(rapidjson::Value *newParent)
    {
        this->jsonParent = newParent;
    }

    void
    setJSONValue(rapidjson::Value *newValue)
    {
        this->jsonValue = newValue;
    }

private:
    // Setting key (i.e. "numThreads")
    const QString key;

    // Setting description (i.e. Number of threads to run the application in)
    QString description;

    // If the setting has been filled with any value other than the default one
    bool filled = false;

    rapidjson::Value *jsonParent = nullptr;
    rapidjson::Value *jsonValue = nullptr;
};

template <typename Type>
class Setting
{
public:
    Setting(const QString &key, const Type &&defaultValue)
        : data(new SettingData<Type>(key, std::move(defaultValue)))
    {
        SettingsManager::registerSetting(this->data);
    }
    Setting(const QString &key)
        : data(new SettingData<Type>(key))
    {
        SettingsManager::registerSetting(this->data);
    }

    Setting &
    setName(const char *newName)
    {
        this->name = newName;

        return *this;
    }

    Setting &
    setParent(Setting<void> *newParent)
    {
        this->parent = newParent;

        return *this;
    }

    const QString &
    getKey() const
    {
        return this->data->getKey();
    }

    const QString &
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

    Setting<void> *parent = nullptr;

    QString name;
};

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

    template <typename JSONType>
    struct JSONWrapper {
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

    template <typename Type>
    static void
    localRegister(std::shared_ptr<SettingData<Type>> setting)
    {
        static_assert(false, "Unimplemented setting type");
    }

    template <>
    static void
    localRegister<void>(std::shared_ptr<SettingData<void>> setting)
    {
        manager->objectSettings.push_back(setting);
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
    std::vector<std::shared_ptr<SettingData<std::string>>> strSettings;
    std::vector<std::shared_ptr<SettingData<double>>> doubleSettings;
    std::vector<std::shared_ptr<SettingData<float>>> floatSettings;
    std::vector<std::shared_ptr<SettingData<void>>> objectSettings;

    static rapidjson::Document *document;

    template <typename Type>
    static rapidjson::Value *
    getSettingParent(std::shared_ptr<SettingData<Type>> &setting)
    {
        auto parent = setting->getParent();
        if (parent == nullptr) {
            return SettingsManager::document;
        }

        return parent;
    }

    bool
    loadObject(const QString & /*objectKey*/)
    {
        // Sanity check
        assert(loaded == true);

// TODO: fix parentage

#if 0
        rapidjson::Value *parent = getSettingParent(setting);

        setting->setJSONParent(SettingsManager::document);

        // Find the key at root level
        if (!parent->IsObject() && !parent->IsArray()) {
            // Parent must be either an object or an array
            std::cerr << "Parent must be either an object or an array"
                      << std::endl;
            return false;
        }

        const QByteArray &tmpArr = setting->getKey().toLatin1();
        const char *settingKey = tmpArr.constData();

        // XXX(pajlada): For now we assume that parents are always objects
        // TODO(pajlada): Implement support for parent arrays
        if (parent->HasMember(settingKey)) {
            /*
            const rapidjson::Value &settingValue = parent[settingKey];

            setting->setJSONValue(
                &const_cast<rapidjson::Value &>(settingValue));

            this->setSetting(setting, settingValue);
            */
        } else {
            rapidjson::Value settingValue;
            settingValue.SetInt(5);

            auto doc = SettingsManager::document;
            auto &allocator = doc->GetAllocator();

            parent->AddMember("asd", settingValue, allocator);

            setting->setJSONValue(
                &const_cast<rapidjson::Value &>(settingValue));

            this->setSetting(setting, settingValue);
        }
#endif

        return false;
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

        const QByteArray &tmpArr = setting->getKey().toLatin1();
        const char *settingKey = tmpArr.constData();

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
    setSetting<int>(std::shared_ptr<SettingData<int>> setting,
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
};

template <>
class Setting<void>
{
public:
    Setting(const QString &key)
        : data(new SettingData<void>(key))
    {
        SettingsManager::registerSetting(this->data);
    }

    Setting &
    setName(const char *newName)
    {
        this->name = newName;

        return *this;
    }

    Setting &
    setParent(Setting<void> *newParent)
    {
        this->parent = newParent;

        return *this;
    }

    const QString &
    getKey() const
    {
        return this->data->getKey();
    }

    const QString &
    getName() const
    {
        return this->name;
    }

private:
    std::shared_ptr<SettingData<void>> data;

    Setting<void> *parent = nullptr;

    QString name;
};

}  // namespace setting
}  // namespace pajadog
