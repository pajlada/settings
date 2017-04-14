#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/settingdata.hpp"

#include <rapidjson/prettywriter.h>

#include <iostream>

using namespace std;

namespace pajlada {
namespace settings {

static constexpr unsigned MAX_ATTEMPTS = 10;

static void mergeObjects(rapidjson::Value &destination,
                         rapidjson::Value &source,
                         rapidjson::Document::AllocatorType &allocator);
static void mergeArrays(rapidjson::Value &destination, rapidjson::Value &source,
                        rapidjson::Document::AllocatorType &allocator);

SettingManager::SettingManager()
    : document(rapidjson::kObjectType)
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
SettingManager::prettyPrintDocument()
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    this->document.Accept(writer);

    cout << buffer.GetString() << endl;
}

void
SettingManager::setPath(const char *path)
{
    manager()->filePath = path;
}

void
SettingManager::clear()
{
    // TODO: what should clear do?
}

bool
SettingManager::load(const char *path)
{
    if (path != nullptr) {
        setPath(path);
    }

    return SettingManager::loadFrom(getPath());
}

template <typename Type>
static inline void
loadSettingsFromVector(SettingManager *manager,
                       vector<shared_ptr<detail::SettingData<Type>>> &vec,
                       unsigned &numSuccessful, unsigned &numFailed)
{
    for (auto it = begin(vec); it != end(vec);) {
        if (manager->loadSetting(*it)) {
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

/* get current working directory
#include <direct.h>
char pBuf[512];
_getcwd(pBuf, 512);
cout << pBuf << endl;
*/

void
mergeObjects(rapidjson::Value &destination, rapidjson::Value &source,
             rapidjson::Document::AllocatorType &allocator)
{
    assert(destination.IsObject());
    assert(source.IsObject());

    for (auto sourceIt = source.MemberBegin(); sourceIt != source.MemberEnd();
         ++sourceIt) {
        // Does the source member exist in destination?
        auto destinationIt = destination.FindMember(sourceIt->name);
        if (destinationIt == destination.MemberEnd()) {
            // Source member was not found in destination. Add member
            destination.AddMember(sourceIt->name, sourceIt->value, allocator);
            continue;
        }

        // Are the source member and destination member with the same key equal
        // types?
        auto sourceType = sourceIt->value.GetType();
        auto destinationType = destinationIt->value.GetType();

        if (sourceType != destinationType) {
            // The types are not the same
            // XXX(pajlada): What do we do in this scenario? For now, we
            // override destination member with source member
            destinationIt->value = sourceIt->value;
            continue;
        }

        // Source type and destination type are equal
        if (sourceType == rapidjson::kObjectType) {
            // Source and destination members are objects. Run mergeObjects
            // I hope we don't recurse our way to hell
            mergeObjects(destinationIt->value, sourceIt->value, allocator);
            continue;
        }

        if (sourceType == rapidjson::kArrayType) {
            // We have multiple options here
            // Do we merge the arrays, index by index? Or do we simply push back
            // any source member indices to the destination member
            // For now, we will merge index by index
            mergeArrays(destinationIt->value, sourceIt->value, allocator);
            continue;
        }

        destinationIt->value = sourceIt->value;
    }
}

void
mergeArrays(rapidjson::Value &destination, rapidjson::Value &source,
            rapidjson::Document::AllocatorType &allocator)
{
    assert(destination.IsArray());
    assert(source.IsArray());

    unsigned index = 0;
    for (auto sourceArrayIt = source.Begin(); sourceArrayIt != source.End();
         ++sourceArrayIt, ++index) {
        if (index < destination.Size()) {
            // Merge
            auto &destinationArrayValue = destination[index];

            auto sourceArrayValueType = sourceArrayIt->GetType();
            auto destinationArrayValueType = destinationArrayValue.GetType();

            if (sourceArrayValueType != destinationArrayValueType) {
                // The types are not the same
                // See comment above for what we should do in this case
                destinationArrayValue = *sourceArrayIt;
                continue;
            }

            if (sourceArrayValueType == rapidjson::kObjectType) {
                mergeObjects(destinationArrayValue, *sourceArrayIt, allocator);
                continue;
            }

            if (sourceArrayValueType == rapidjson::kArrayType) {
                mergeArrays(destinationArrayValue, *sourceArrayIt, allocator);
                continue;
            }

            // Same type, just replace the value
            destinationArrayValue = *sourceArrayIt;
        } else {
            // Append
            destination.PushBack(*sourceArrayIt, allocator);
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

    rapidjson::Document d;

    d.Parse(fileBuffer, fileSize);

    // Close file

    fclose(fh);
    delete[] fileBuffer;

    // This restricts config files a bit. They NEED to have an object root
    if (!d.IsObject()) {
        return false;
    }

    // Merge newly parsed config file into our pre-existing document
    // The pre-existing document might be empty, but we don't know that
    auto &document = manager()->document;

    // Perform deep merge of objects
    mergeObjects(document, d, document.GetAllocator());

    // Fill in any settings that registered before we called load
    // Make a copy of the lists of settings we want to load
    unsigned numAttempts = 0;
    unsigned numFailed = 0;
    unsigned numSuccessful = 0;
    auto objectSettings = manager()->objectSettings;
    auto arraySettings = manager()->arraySettings;
    auto intSettings = manager()->intSettings;
    auto floatSettings = manager()->floatSettings;
    auto doubleSettings = manager()->doubleSettings;
    auto strSettings = manager()->strSettings;
    auto boolSettings = manager()->boolSettings;
    do {
        numFailed = 0;
        numSuccessful = 0;
        loadSettingsFromVector(manager(), objectSettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(manager(), arraySettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(manager(), intSettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(manager(), floatSettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(manager(), doubleSettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(manager(), strSettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(manager(), boolSettings, numSuccessful,
                               numFailed);
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
        setPath(path);
    }

    return SettingManager::saveAs(getPath());
}

bool
SettingManager::saveAs(const char *path)
{
    FILE *fh = fopen(path, "wb+");
    if (fh == nullptr) {
        // Unable to open file at `path`
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    manager()->document.Accept(writer);

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
SettingManager::setSetting<Object>(shared_ptr<detail::SettingData<Object>>,
                                   const rapidjson::Value &)
{
    // Do nothing
    // Void = object type
    return true;
}

template <>
bool
SettingManager::setSetting<Array>(shared_ptr<detail::SettingData<Array>>,
                                  const rapidjson::Value &)
{
    // Do nothing
    // Void = object type
    return true;
}

template <>
bool
SettingManager::setSetting<float>(
    shared_ptr<detail::SettingData<float>> setting,
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
SettingManager::setSetting<double>(
    shared_ptr<detail::SettingData<double>> setting,
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
SettingManager::setSetting<string>(
    shared_ptr<detail::SettingData<string>> setting,
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
SettingManager::setSetting<bool>(shared_ptr<detail::SettingData<bool>> setting,
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
SettingManager::setSetting<int>(shared_ptr<detail::SettingData<int>> setting,
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
SettingManager::getSettingParent(shared_ptr<detail::SettingData<Type>> &setting)
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

rapidjson::Document &
SettingManager::getDocument()
{
    return manager()->document;
}

}  // namespace settings
}  // namespace pajlada
