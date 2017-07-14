#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/merger.hpp"
#include "pajlada/settings/serialize.hpp"
#include "pajlada/settings/settingdata.hpp"

#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>

#include <iostream>
#include <string>

using namespace std;

namespace pajlada {
namespace Settings {

SettingManager::SettingManager()
    : document(rapidjson::kObjectType)
{
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
SettingManager::pp(const string &prefix)
{
    SettingManager::ppDocument(SettingManager::getInstance().document, prefix);
}

void
SettingManager::ppDocument(const rapidjson::Document &_document,
                           const string &prefix)
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    _document.Accept(writer);

    cout << prefix << buffer.GetString() << endl;
}

std::string
SettingManager::stringify(const rapidjson::Value &v)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);

    return std::string(buffer.GetString());
}

rapidjson::Value *
SettingManager::rawValue(const char *path)
{
    SettingManager &instance = SettingManager::getInstance();

    return rapidjson::Pointer(path).Get(instance.document);
}

rapidjson::SizeType
SettingManager::arraySize(const string &path)
{
    SettingManager &instance = SettingManager::getInstance();

    auto valuePointer = rapidjson::Pointer(path.c_str()).Get(instance.document);
    if (valuePointer == nullptr) {
        return false;
    }

    rapidjson::Value &value = *valuePointer;

    if (!value.IsArray()) {
        // Do we need to throw an error here?
        return 0;
    }

    return value.Size();
}

// Returns true if the value at the given path is null or if doesn't exist
bool
SettingManager::isNull(const std::string &path)
{
    SettingManager &instance = SettingManager::getInstance();

    return instance._isNull(path);
}

bool
SettingManager::_isNull(const std::string &path)
{
    auto valuePointer = rapidjson::Pointer(path.c_str()).Get(this->document);
    if (valuePointer == nullptr) {
        return true;
    }

    return valuePointer->IsNull();
}

void
SettingManager::setNull(const std::string &path)
{
    SettingManager &instance = SettingManager::getInstance();

    rapidjson::Pointer(path.c_str()).Set(instance.document, rapidjson::Value());
}

bool
SettingManager::removeArrayValue(const std::string &arrayPath,
                                 rapidjson::SizeType index)
{
    SettingManager &instance = SettingManager::getInstance();

    instance.clearSettings(arrayPath + "/" + std::to_string(index) + "/");

    rapidjson::SizeType size = SettingManager::arraySize(arrayPath);

    if (size == 0) {
        // No values to remove
        return false;
    }

    if (index >= size) {
        // Index out of bounds
        return false;
    }

    auto valuePointer =
        rapidjson::Pointer(arrayPath.c_str()).Get(instance.document);
    if (valuePointer == nullptr) {
        return false;
    }

    rapidjson::Value &array = *valuePointer;

    if (index == size - 1) {
        // We want to remove the last element
        array.PopBack();
    } else {
        SettingManager::setNull(arrayPath + "/" + std::to_string(index));
    }

    instance.clearSettings(arrayPath + "/" + std::to_string(index) + "/");

    return true;
}

rapidjson::SizeType
SettingManager::cleanArray(const std::string &arrayPath)
{
    rapidjson::SizeType size = SettingManager::arraySize(arrayPath);

    if (size == 0) {
        // No values to remove
        return 0;
    }

    SettingManager &instance = SettingManager::getInstance();

    rapidjson::SizeType numValuesRemoved = 0;

    for (rapidjson::SizeType i = size - 1; i > 0; --i) {
        if (instance._isNull(arrayPath + "/" + std::to_string(i))) {
            SettingManager::removeArrayValue(arrayPath, i);
            ++numValuesRemoved;
        }
    }

    return numValuesRemoved;
}

void
SettingManager::clear()
{
    SettingManager &instance = SettingManager::getInstance();

    // Clear document
    rapidjson::Value(rapidjson::kObjectType).Swap(instance.document);

    // Clear map of settings
    lock_guard<mutex> lock(instance.settingsMutex);

    instance.settings.clear();
}

void
SettingManager::registerSetting(shared_ptr<ISettingData> &setting)
{
    // Save initial value
    // We might want to have this as a setting?
    // TODO: Re-implement this
    setting->marshalInto(this->document);

    // Set up a signal which updates the rapidjson document with the new
    // value when the SettingData value is updated
    setting->registerDocument(this->document);

    // file loaded with SettingManager, this callback will also fire. the
    // only bad part about that is that the setValue method is called
    // unnecessarily

    // Load value from currently loaded document
    setting->unmarshalFrom(this->document);
}

void
SettingManager::clearSettings(const std::string &root)
{
    std::lock_guard<std::mutex> lock(this->settingsMutex);

    std::vector<std::string> keysToBeRemoved;

    for (const auto &setting : this->settings) {
        if (setting.first.compare(0, root.length(), root) == 0) {
            keysToBeRemoved.push_back(setting.first);
        }
    }

    for (const auto &settingKey : keysToBeRemoved) {
        this->settings.erase(settingKey);
    }
}

SettingManager::LoadError
SettingManager::load(const char *path)
{
    SettingManager &instance = SettingManager::getInstance();

    if (path != nullptr) {
        instance.filePath = path;
    }

    return SettingManager::loadFrom(instance.filePath.c_str());
}

/* get current working directory
#include <direct.h>
char pBuf[512];
_getcwd(pBuf, 512);
cout << pBuf << endl;
*/

SettingManager::LoadError
SettingManager::loadFrom(const char *path)
{
    SettingManager &instance = SettingManager::getInstance();

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

    if (readBytes != static_cast<size_t>(fileSize)) {
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

    rapidjson::ParseResult ok = instance.document.Parse(fileBuffer, fileSize);

    // Make sure the file parsed okay
    if (!ok) {
        return LoadError::JSONParseError;
    }

    // This restricts config files a bit. They NEED to have an object root
    if (!instance.document.IsObject()) {
        return LoadError::JSONParseError;
    }

    // Perform deep merge of objects
    // detail::mergeObjects(document, d, document.GetAllocator());

    {
        // Fill in any settings that registered before we called load
        instance.settingsMutex.lock();

        auto settingsCopy = instance.settings;

        instance.settingsMutex.unlock();

        for (const auto &it : settingsCopy) {
            const shared_ptr<ISettingData> &setting = it.second;

            setting->unmarshalFrom(instance.document);
        }
    }

    return LoadError::NoError;
}

bool
SettingManager::save(const char *path)
{
    SettingManager &instance = SettingManager::getInstance();

    if (path != nullptr) {
        instance.filePath = path;
    }

    return SettingManager::saveAs(instance.filePath.c_str());
}

bool
SettingManager::saveAs(const char *path)
{
    PS_DEBUG("Saving to " << path);

    SettingManager &instance = SettingManager::getInstance();

    {
        lock_guard<mutex> lock(instance.settingsMutex);

        for (const auto &it : instance.settings) {
            const shared_ptr<ISettingData> &setting = it.second;

            if (setting->needsMarshalling) {
                setting->marshal(instance.document);
            }
        }
    }

    FILE *fh = fopen(path, "wb+");
    if (fh == nullptr) {
        // Unable to open file at `path`
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    instance.document.Accept(writer);

    auto writtenBytes = fwrite(buffer.GetString(), 1, buffer.GetSize(), fh);

    // Close file handle
    fclose(fh);

    if (writtenBytes != buffer.GetSize()) {
        // Something went wrong with saving the file;
        return false;
    }

    return true;
}

}  // namespace Settings
}  // namespace pajlada
