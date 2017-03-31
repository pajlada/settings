#pragma once

#include <rapidjson/document.h>
#include <QString>

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace pajadog {
namespace settings {

template <typename Type>
class SettingData
{
public:
    SettingData(const QString &_key, const Type &&defaultValue)
        : key(_key)
        , data(defaultValue)
    {
    }

    ~SettingData()
    {
    }

    SettingData<Type> *
    setDescription(const QString &newDescription)
    {
        this->description = newDescription;

        return this;
    }

    void
    setValue(const Type &newValue)
    {
        this->data = newValue;

        this->filled = true;

        if (this->jsonValue != nullptr) {
            this->setJSONValueValue(newValue);
        }
    }

    template <typename JSONType>
    void
    setJSONValueValue(const JSONType &newValue)
    {
        std::cout << "generic json value setter" << std::endl;
    }

    template <>
    void
    setJSONValueValue<int>(const int &newValue)
    {
        std::cout << "int specialization" << std::endl;
        this->jsonValue->SetInt(newValue);
    }

    template <>
    void
    setJSONValueValue<double>(const double &newValue)
    {
        this->jsonValue->SetDouble(newValue);
    }

    template <>
    void
    setJSONValueValue<float>(const float &newValue)
    {
        this->jsonValue->SetFloat(newValue);
    }

    template <>
    void
    setJSONValueValue<std::string>(const std::string &newValue)
    {
        this->jsonValue->SetString(newValue.c_str(), SettingsManager::document.GetAllocator());
    }

    Type
    getValue() const
    {
        return this->data;
    }

    Type data;

    virtual int
    getInt()
    {
        return 0;
    }

    QString
    toString() const
    {
        QString ret;

        if (!this->key.isEmpty()) {
            ret += "Key: " + this->key + "\n";
        }
        if (!this->description.isEmpty()) {
            ret += "Description: " + this->description + "\n";
        }
        try {
            ret += QString("Value: %1\n").arg(this->getValue<int>());
        }

        return ret.trimmed();
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

class ISetting
{
};

template <typename Type>
class Setting : public ISetting
{
public:
    Setting(const QString &key, const Type &&defaultValue)
        : data(new SettingData<Type>(key, std::move(defaultValue)))
    {
        SettingsManager::registerSetting(this->data);
    }

    const Type
    getValue() const
    {
        assert(this->data != nullptr);

        return this->data->getValue();
    }

    void
    setValue(const Type &newValue)
    {
        this->data->setValue(newValue);
    }

private:
    std::shared_ptr<SettingData<Type>> data;
};

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

    std::string path = "settings.json";

    template <typename Type>
    static void
    registerSetting(std::shared_ptr<SettingData<Type>> setting)
    {
        if (loaded) {
            // If settings are already loaded from a file, try to fill in the
            // settings
            // manager.loadSetting(setting);
        }

        SettingsManager::localRegister(std::move(setting));
        // manager.settings.push_back(std::ref(setting));
    }

    template <typename Type>
    static void
    localRegister(std::shared_ptr<SettingData<Type>> setting)
    {
        static_assert(false, "xd");
    }

    template <>
    static void
    localRegister<int>(std::shared_ptr<SettingData<int>> setting)
    {
        manager.intSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<std::string>(
        std::shared_ptr<SettingData<std::string>> setting)
    {
        manager.strSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<float>(std::shared_ptr<SettingData<float>> setting)
    {
        manager.floatSettings.push_back(setting);
    }

    template <>
    static void
    localRegister<double>(std::shared_ptr<SettingData<double>> setting)
    {
        manager.doubleSettings.push_back(setting);
    }

    static SettingsManager manager;

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

    std::vector<std::reference_wrapper<ISetting>> settings;
    std::vector<std::shared_ptr<SettingData<int>>> intSettings;
    std::vector<std::shared_ptr<SettingData<std::string>>> strSettings;
    std::vector<std::shared_ptr<SettingData<double>>> doubleSettings;
    std::vector<std::shared_ptr<SettingData<float>>> floatSettings;

    static rapidjson::Document document;

    template <typename Type>
    static const rapidjson::Value &
    getSettingParent(std::shared_ptr<SettingData<Type>> &setting)
    {
        /*
        if (setting->getParent() == nullptr) {
            return SettingsManager::document;
        }
        */

        // TODO(pajlada): Get parent object
        return SettingsManager::document;
    }

    template <typename Type>
    bool
    loadSetting(std::shared_ptr<SettingData<Type>> setting)
    {
        // Sanity check
        assert(loaded == true);

        // TODO: fix parentage

        const rapidjson::Value &parent = getSettingParent(setting);

        setting->setJSONParent(&SettingsManager::document);

        // Find the key at root level
        if (!parent.IsObject() && !parent.IsArray()) {
            // Parent must be either an object or an array
            std::cerr << "Parent must be either an object or an array"
                      << std::endl;
            return false;
        }

        const QByteArray &tmpArr = setting->getKey().toLatin1();
        const char *settingKey = tmpArr.constData();

        // XXX(pajlada): For now we assume that parents are always objects
        // TODO(pajlada): Implement support for parent arrays
        if (parent.HasMember(settingKey)) {
            const rapidjson::Value &settingValue = parent[settingKey];

            setting->setJSONValue(
                &const_cast<rapidjson::Value &>(settingValue));

            this->setSetting(setting, settingValue);
        }

        return false;
    }

    template <typename Type>
    bool
    setSetting(std::shared_ptr<SettingData<Type>> setting,
               const rapidjson::Value &value)
    {
        std::cout << "set setting base" << std::endl;

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

            case rapidjson::Type::kStringType: {
                std::string str = value.GetString();
                setting->setValue(str);
                return true;
            } break;
        }

        return false;
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
};

}  // namespace setting
}  // namespace pajadog
