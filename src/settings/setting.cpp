#include "pajlada/settings/setting.hpp"

#include <rapidjson/prettywriter.h>

#include <iostream>
#include <vector>

namespace pajlada {
namespace settings {

static constexpr unsigned MAX_ATTEMPTS = 10;

rapidjson::Document *SettingsManager::document = nullptr;
std::atomic<uint64_t> ISettingData::latestConnectionID = 0;

SettingsManager *SettingsManager::manager = nullptr;
bool SettingsManager::loaded = false;

SettingsManager::SettingsManager()
{
    // XXX(pajlada): Load "default.json"?
}

SettingsManager::~SettingsManager()
{
    // XXX(pajlada): Should settings automatically save on exit?
    // Or on each setting change?
    // Or only manually?
    if (this->checkSaveMethodFlag(SaveMethod::SaveOnExitFlag)) {
        SettingsManager::save();
    }
}

void
SettingsManager::setPath(const char *path)
{
    manager->path = path;
}

void
SettingsManager::clear()
{
    delete document;

    document = nullptr;
    loaded = false;
}

bool
SettingsManager::load(const char *path)
{
    if (path != nullptr) {
        manager->path = path;
    }

    return SettingsManager::loadFrom(manager->path.c_str());
}

template <typename Type>
static inline void
loadSettingsFromVector(std::vector<std::shared_ptr<SettingData<Type>>> &vec,
                       unsigned &numSuccessful, unsigned &numFailed)
{
    for (auto it = std::begin(vec); it != std::end(vec);) {
        if (SettingsManager::manager->loadSetting(*it)) {
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
SettingsManager::loadFrom(const char *path)
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
SettingsManager::save(const char *path)
{
    if (path != nullptr) {
        manager->path = path;
    }

    return SettingsManager::saveAs(manager->path.c_str());
}

bool
SettingsManager::saveAs(const char *path)
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

}  // namespace settings
}  // namespace pajlada
