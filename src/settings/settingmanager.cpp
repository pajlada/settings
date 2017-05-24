#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/settingdata.hpp"

#include <rapidjson/prettywriter.h>

#include <iostream>

using namespace std;

namespace pajlada {
namespace Settings {

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

SettingManager::LoadError
SettingManager::load(const char *path)
{
    if (path != nullptr) {
        setPath(path);
    }

    return SettingManager::loadFrom(manager()->filePath.c_str());
}

template <typename Type>
static inline void
loadSettingsFromVector(rapidjson::Document &document,
                       vector<shared_ptr<detail::SettingData<Type>>> &vec,
                       unsigned &numSuccessful, unsigned &numFailed)
{
    for (auto it = begin(vec); it != end(vec);) {
        if (detail::loadSetting(document, *it)) {
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

SettingManager::LoadError
SettingManager::loadFrom(const char *path)
{
    // Open file
    FILE *fh = fopen(path, "rb");
    if (fh == nullptr) {
        // Unable to open file at `path`
        return LoadError::CannotOpenFile;
    }

    // Read size of file
    if (fseek(fh, 0, SEEK_END) != 0) {
        return LoadError::FileSeekError;
    }

    auto fileSize = ftell(fh);
    if (fileSize == -1L) {
        // An error occured when ftelling
        return LoadError::FileHandleError;
    }
    fseek(fh, 0, SEEK_SET);

    if (fileSize == 0) {
        // Nothing to load
        return LoadError::NoError;
    }

    // Create vector of appropriate size
    char *fileBuffer = new char[fileSize];

    // Read file data into buffer
    auto readBytes = fread(fileBuffer, 1, fileSize, fh);

    if (readBytes != static_cast<std::size_t>(fileSize)) {
        // Error reading the buffer
        fclose(fh);
        delete[] fileBuffer;

        return LoadError::FileReadError;
    }

    // Close file
    fclose(fh);

    // XXX: Temporarily don't delete the buffer
    // delete[] fileBuffer;

    // Merge newly parsed config file into our pre-existing document
    // The pre-existing document might be empty, but we don't know that
    auto &document = manager()->document;

    rapidjson::ParseResult ok = document.Parse(fileBuffer, fileSize);

    // Make sure the file parsed okay
    if (!ok) {
        return LoadError::JSONParseError;
    }

    // This restricts config files a bit. They NEED to have an object root
    if (!document.IsObject()) {
        return LoadError::JSONParseError;
    }

    // Perform deep merge of objects
    // mergeObjects(document, d, document.GetAllocator());

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
        loadSettingsFromVector(document, objectSettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(document, arraySettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(document, intSettings, numSuccessful, numFailed);
        loadSettingsFromVector(document, floatSettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(document, doubleSettings, numSuccessful,
                               numFailed);
        loadSettingsFromVector(document, strSettings, numSuccessful, numFailed);
        loadSettingsFromVector(document, boolSettings, numSuccessful,
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

    return LoadError::NoError;
}

bool
SettingManager::save(const char *path)
{
    if (path != nullptr) {
        setPath(path);
    }

    return SettingManager::saveAs(manager()->filePath.c_str());
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

rapidjson::Document &
SettingManager::getDocument()
{
    return manager()->document;
}

template <>
void
SettingManager::localRegister<Array>(
    std::shared_ptr<detail::SettingData<Array>> setting)
{
    manager()->arraySettings.push_back(setting);
}

template <>
void
SettingManager::localRegister<Object>(
    std::shared_ptr<detail::SettingData<Object>> setting)
{
    manager()->objectSettings.push_back(setting);
}

template <>
void
SettingManager::localRegister<bool>(
    std::shared_ptr<detail::SettingData<bool>> setting)
{
    manager()->boolSettings.push_back(setting);
}

template <>
void
SettingManager::localRegister<int>(
    std::shared_ptr<detail::SettingData<int>> setting)
{
    manager()->intSettings.push_back(setting);
}

template <>
void
SettingManager::localRegister<std::string>(
    std::shared_ptr<detail::SettingData<std::string>> setting)
{
    manager()->strSettings.push_back(setting);
}

template <>
void
SettingManager::localRegister<float>(
    std::shared_ptr<detail::SettingData<float>> setting)
{
    manager()->floatSettings.push_back(setting);
}

template <>
void
SettingManager::localRegister<double>(
    std::shared_ptr<detail::SettingData<double>> setting)
{
    manager()->doubleSettings.push_back(setting);
}

template <>
void
SettingManager::localUnregister<Array>(
    const std::shared_ptr<detail::SettingData<Array>> &setting)
{
    SettingManager::removeSettingFrom(manager()->arraySettings, setting);
}

template <>
void
SettingManager::localUnregister<Object>(
    const std::shared_ptr<detail::SettingData<Object>> &setting)
{
    SettingManager::removeSettingFrom(manager()->objectSettings, setting);
}

template <>
void
SettingManager::localUnregister<bool>(
    const std::shared_ptr<detail::SettingData<bool>> &setting)
{
    SettingManager::removeSettingFrom(manager()->boolSettings, setting);
}

template <>
void
SettingManager::localUnregister<int>(
    const std::shared_ptr<detail::SettingData<int>> &setting)
{
    SettingManager::removeSettingFrom(manager()->intSettings, setting);
}

template <>
void
SettingManager::localUnregister<std::string>(
    const std::shared_ptr<detail::SettingData<std::string>> &setting)
{
    SettingManager::removeSettingFrom(manager()->strSettings, setting);
}

template <>
void
SettingManager::localUnregister<float>(
    const std::shared_ptr<detail::SettingData<float>> &setting)
{
    SettingManager::removeSettingFrom(manager()->floatSettings, setting);
}

template <>
void
SettingManager::localUnregister<double>(
    const std::shared_ptr<detail::SettingData<double>> &setting)
{
    SettingManager::removeSettingFrom(manager()->doubleSettings, setting);
}

namespace detail {

template <>
void
setValueSoft(rapidjson::Document &document, const char *path, const Object &)
{
    // XXX: not sure if this is soft enough
    // rapidjson::Pointer(path).Create(document);
}

template <>
void
setValueSoft(rapidjson::Document &document, const char *path, const Array &)
{
    // XXX: not sure if this is soft enough
    // rapidjson::Pointer(path).Create(document);
}

template <>
void
setValue(rapidjson::Document &document, const char *path,
         const std::string &value)
{
    rapidjson::Pointer(path).Set(document, value.c_str());
}

template <>
void
setValue(rapidjson::Document &document, const char *path, const Object &)
{
    rapidjson::Pointer(path).Create(document);
}

template <>
void
setValue(rapidjson::Document &document, const char *path, const Array &)
{
    rapidjson::Pointer(path).Create(document);
}

template <>
bool
setSetting(shared_ptr<SettingData<Object>>, const rapidjson::Value &)
{
    // Do nothing
    return true;
}

template <>
bool
setSetting(shared_ptr<SettingData<Array>>, const rapidjson::Value &)
{
    // Do nothing
    return true;
}

template <>
bool
setSetting<float>(shared_ptr<SettingData<float>> setting,
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

        default: {
            // We should never get here
            // If we do, then we shouldn't do anything
        } break;
    }

    return false;
}

template <>
bool
setSetting<double>(shared_ptr<SettingData<double>> setting,
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

        default: {
            // We should never get here
            // If we do, then we shouldn't do anything
        } break;
    }

    return false;
}

template <>
bool
setSetting<string>(shared_ptr<SettingData<string>> setting,
                   const rapidjson::Value &value)
{
    auto type = value.GetType();

    switch (type) {
        case rapidjson::Type::kStringType: {
            setting->setValue(value.GetString());
            return true;
        } break;

        default: {
            // We should never get here
            // If we do, then we shouldn't do anything
        } break;
    }

    return false;
}

template <>
bool
setSetting<bool>(shared_ptr<SettingData<bool>> setting,
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

        default: {
            // We should never get here
            // If we do, then we shouldn't do anything
        } break;
    }

    return false;
}

template <>
bool
setSetting<int>(shared_ptr<SettingData<int>> setting,
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

        default: {
            // We should never get here
            // If we do, then we shouldn't do anything
        } break;
    }

    return false;
}

template <typename Type>
bool
loadSetting(rapidjson::Document &document,
            std::shared_ptr<SettingData<Type>> &setting)
{
    // A setting should always have a path
    assert(!setting->getPath().empty());

    return loadSettingFromPath(document, setting);
}

template <>
bool
loadSetting(rapidjson::Document &, std::shared_ptr<SettingData<Object>> &)
{
    return true;
}

template <>
bool
loadSetting(rapidjson::Document &, std::shared_ptr<SettingData<Array>> &)
{
    return true;
}

template <typename Type>
bool
loadSettingFromPath(rapidjson::Document &document,
                    std::shared_ptr<SettingData<Type>> &setting)
{
    const char *path = setting->getPath().c_str();
    auto value = rapidjson::Pointer(path).Get(document);
    if (value == nullptr) {
        return false;
    }

    setSetting(setting, *value);

    return true;
}

}  // namespace detail

}  // namespace Settings
}  // namespace pajlada
