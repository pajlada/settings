#include "pajlada/settings/settingmanager.hpp"
#include "pajlada/settings/merger.hpp"
#include "pajlada/settings/serialize.hpp"
#include "pajlada/settings/settingdata.hpp"

#include <rapidjson/prettywriter.h>

#include <iostream>

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
SettingManager::pp()
{
    SettingManager::ppDocument(SettingManager::getInstance().document);
}

void
SettingManager::ppDocument(const rapidjson::Document &_document)
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    _document.Accept(writer);

    cout << buffer.GetString() << endl;
}

void
SettingManager::registerSetting(std::shared_ptr<ISettingData> &setting)
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
            const std::shared_ptr<ISettingData> &setting = it.second;

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
    SettingManager &instance = SettingManager::getInstance();

    {
        // Update any dirty settings
        std::lock_guard<std::mutex> lock(instance.settingsMutex);

        for (const auto &it : instance.settings) {
            const std::shared_ptr<ISettingData> &setting = it.second;

            if (setting->dirty || setting->isFilled()) {
                setting->marshal(instance.document);
                // setting->marshalInto(instance.document);
                setting->dirty = false;
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
