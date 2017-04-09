#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/setting.hpp"
#include "pajlada/settings/settingdata.hpp"

#include <rapidjson/prettywriter.h>

#include <iostream>
#include <vector>

namespace pajlada {
namespace settings {

static constexpr unsigned MAX_ATTEMPTS = 10;

SettingManager::SettingManager()
{
    // XXX(pajlada): Load "default.json"?
}

SettingManager::~SettingManager()
{
    // XXX(pajlada): Should settings automatically save on exit?
    // Or on each setting change?
    // Or only manually?
    if (this->checkSaveMethodFlag(SaveMethod::SaveOnExitFlag)) {
        SettingManager::save();
    }
}

void
SettingManager::setPath(const char *path)
{
    manager->path = path;
}

void
SettingManager::clear()
{
    delete document;

    document = nullptr;
    loaded = false;
}

bool
SettingManager::load(const char *path)
{
    if (path != nullptr) {
        manager->path = path;
    }

    return SettingManager::loadFrom(manager->path.c_str());
}

template <typename Type>
static inline void
loadSettingsFromVector(std::vector<std::shared_ptr<SettingData<Type>>> &vec,
                       unsigned &numSuccessful, unsigned &numFailed)
{
    for (auto it = std::begin(vec); it != std::end(vec);) {
        if (SettingManager::manager->loadSetting(*it)) {
            // Setting successfully loaded
            ++numSuccessful;
            it = vec.erase(it);
        } else {
            // Setting failed to load
            ++numFailed;
            ++it;
        }
    }
}

bool
SettingManager::loadFrom(const char *path)
{
    // Open file
    FILE *fh = fopen(path, "rb");
    if (fh == nullptr) {
        // Unable to open file at `path`
        return false;
    }

    // Read size of file
    fseek(fh, 0, SEEK_END);
    auto fileSize = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    // Create vector of appropriate size
    char *fileBuffer = new char[fileSize];

    // Read file data into buffer
    auto readBytes = fread(fileBuffer, 1, fileSize, fh);

    if (readBytes != fileSize) {
        // Error reading the buffer
        fclose(fh);
        delete[] fileBuffer;

        return false;
    }

    auto d = new rapidjson::Document;

    d->Parse(fileBuffer, fileSize);

    // Close file

    fclose(fh);
    delete[] fileBuffer;

    // This restricts config files a bit. They NEED to have an object root
    if (!d->IsObject()) {
        return false;
    }

    if (loaded) {
        // A document has already been created/loaded
        // Merge newly parsed config file into our pre-existing document
        document->CopyFrom(*d, document->GetAllocator());

        delete d;
    } else {
        document = d;
    }

    loaded = true;

    // Fill in any settings that registered before we called load
    // Make a copy of the lists of settings we want to load
    unsigned numAttempts = 0;
    unsigned numFailed = 0;
    unsigned numSuccessful = 0;
    auto objectSettings = manager->objectSettings;
    auto arraySettings = manager->arraySettings;
    auto intSettings = manager->intSettings;
    auto floatSettings = manager->floatSettings;
    auto doubleSettings = manager->doubleSettings;
    auto strSettings = manager->strSettings;
    auto boolSettings = manager->boolSettings;
    do {
        std::cout << "Attempt " << numAttempts + 1 << std::endl;
        numFailed = 0;
        numSuccessful = 0;
        loadSettingsFromVector(objectSettings, numSuccessful, numFailed);
        loadSettingsFromVector(arraySettings, numSuccessful, numFailed);
        loadSettingsFromVector(intSettings, numSuccessful, numFailed);
        loadSettingsFromVector(floatSettings, numSuccessful, numFailed);
        loadSettingsFromVector(doubleSettings, numSuccessful, numFailed);
        loadSettingsFromVector(strSettings, numSuccessful, numFailed);
        loadSettingsFromVector(boolSettings, numSuccessful, numFailed);
        // Retry if:
        // One or more settings failed to load
        // AND
        // One more more settings successfully loaded (otherwise there's no
        // chance at resolution)
        // AND
        // We haven't bypassed the three tries
    } while (++numAttempts < MAX_ATTEMPTS && numFailed > 0 &&
             numSuccessful > 0);

    return true;
}

bool
SettingManager::save(const char *path)
{
    if (path != nullptr) {
        manager->path = path;
    }

    return SettingManager::saveAs(manager->path.c_str());
}

bool
SettingManager::saveAs(const char *path)
{
    if (!loaded) {
        return false;
    }

    FILE *fh = fopen(path, "wb+");
    if (fh == nullptr) {
        // Unable to open file at `path`
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document->Accept(writer);

    std::cout << "save document" << std::endl;

    auto writtenBytes = fwrite(buffer.GetString(), 1, buffer.GetSize(), fh);

    // Close file handle
    fclose(fh);

    if (writtenBytes != buffer.GetSize()) {
        // Something went wrong with saving the file;
        return false;
    }

    return true;
}

template <>
bool
SettingManager::setSetting<Object>(std::shared_ptr<SettingData<Object>>,
                                   const rapidjson::Value &)
{
    // Do nothing
    // Void = object type
    return true;
}

template <>
bool
SettingManager::setSetting<Array>(std::shared_ptr<SettingData<Array>>,
                                  const rapidjson::Value &)
{
    // Do nothing
    // Void = object type
    return true;
}

template <>
bool
SettingManager::setSetting<float>(std::shared_ptr<SettingData<float>> setting,
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
SettingManager::setSetting<double>(std::shared_ptr<SettingData<double>> setting,
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
SettingManager::setSetting<std::string>(
    std::shared_ptr<SettingData<std::string>> setting,
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
SettingManager::setSetting<bool>(std::shared_ptr<SettingData<bool>> setting,
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
SettingManager::setSetting<int>(std::shared_ptr<SettingData<int>> setting,
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

template <typename Type>
rapidjson::Value *
SettingManager::getSettingParent(std::shared_ptr<SettingData<Type>> &setting)
{
    auto settingObjParent = setting->getSettingObjectParent();
    auto settingArrParent = setting->getSettingArrayParent();

    if (settingObjParent == nullptr && settingArrParent == nullptr) {
        // No parent set
        return SettingManager::document;
    }

    if (settingObjParent != nullptr) {
        // Has the setting parent been loaded/created yet? (does it have a
        // jsonValue yet?)
        auto parentData = settingObjParent->getData();

        if (parentData->getJSONValue() != nullptr) {
            return parentData->getJSONValue();
        }
    } else if (settingArrParent != nullptr) {
        // Has the setting parent been loaded/created yet? (does it have a
        // jsonValue yet?)
        auto parentData = settingArrParent->getData();

        if (parentData->getJSONValue() != nullptr) {
            return parentData->getJSONValue();
        }
    }

    // returning nullptr means that we should give this another pass
    return nullptr;
}

}  // namespace settings
}  // namespace pajlada
