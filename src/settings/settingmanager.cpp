#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>

#include <fstream>
#include <iostream>
#include <pajlada/settings/backup.hpp>
#include <pajlada/settings/detail/realpath.hpp>
#include <pajlada/settings/internal.hpp>
#include <pajlada/settings/settingdata.hpp>
#include <pajlada/settings/settingmanager.hpp>
#include <string>

namespace pajlada::Settings {

SettingManager::SettingManager()
    : document(rapidjson::kObjectType)
{
}

SettingManager::~SettingManager()
{
    // XXX(pajlada): Should settings automatically save on exit?
    // Or on each setting change?
    // Or only manually?
    if (this->hasSaveMethodFlag(SaveMethod::SaveOnExit)) {
        this->save();
    }
}

rapidjson::Value *
SettingManager::get(const char *path)
{
    auto ptr = rapidjson::Pointer(path);

    if (!ptr.IsValid()) {
        // For invalid paths, i.e. "988934jksgrhjkh" or "jgkh34gjk" (missing /)
        return nullptr;
    }

    return ptr.Get(this->document);
}

bool
SettingManager::set(const char *path, const rapidjson::Value &value,
                    SignalArgs args)
{
    if (args.compareBeforeSet) {
        const auto *prevValue = rapidjson::Pointer(path).Get(this->document);
        if (prevValue != nullptr && *prevValue == value) {
            return false;
        }
    }

    this->hasUnsavedChanges = true;

    if (args.writeToFile) {
        rapidjson::Pointer(path).Set(this->document, value);

        if (this->hasSaveMethodFlag(SaveMethod::SaveOnSettingChange)) {
            this->save();
        }
    }

    this->notifyUpdate(path, value, std::move(args));

    return true;
}

void
SettingManager::notifyUpdate(const std::string &path,
                             const rapidjson::Value &value, SignalArgs args)
{
    auto setting = this->getSetting(path);
    if (!setting) {
        return;
    }

    setting->notifyUpdate(value, std::move(args));
}

void
SettingManager::notifyLoadedValues()
{
    // Fill in any settings that registered before we called load
    this->settingsMutex.lock();

    auto loadedSettings = this->settings;

    this->settingsMutex.unlock();

    for (const auto &it : loadedSettings) {
        auto *v = this->get(it.first.c_str());
        if (v == nullptr) {
            continue;
        }

        // Maybe a "Load" source would make sense?
        SignalArgs args;
        args.source = SignalArgs::Source::Setter;

        it.second->notifyUpdate(*v, std::move(args));
    }
}

rapidjson::SizeType
SettingManager::arraySize(const std::string &path)
{
    return SettingManager::arraySize(path, SettingManager::getInstance());
}

rapidjson::SizeType
SettingManager::arraySize(const std::string &path,
                          std::shared_ptr<SettingManager> instance)
{
    auto *valuePointer =
        rapidjson::Pointer(path.c_str()).Get(instance->document);
    if (valuePointer == nullptr) {
        return 0;
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
    const auto &instance = SettingManager::getInstance();

    return instance->_isNull(path);
}

bool
SettingManager::_isNull(const std::string &path)
{
    auto *valuePointer = rapidjson::Pointer(path.c_str()).Get(this->document);
    if (valuePointer == nullptr) {
        return true;
    }

    return valuePointer->IsNull();
}

void
SettingManager::setNull(const std::string &path)
{
    const auto &instance = SettingManager::getInstance();

    rapidjson::Pointer(path.c_str())
        .Set(instance->document, rapidjson::Value());
}

bool
SettingManager::removeArrayValue(const std::string &arrayPath,
                                 rapidjson::SizeType index)
{
    const auto &instance = SettingManager::getInstance();

    instance->clearSettings(arrayPath + "/" + std::to_string(index) + "/");

    rapidjson::SizeType size = SettingManager::arraySize(arrayPath);

    if (size == 0) {
        // No values to remove
        return false;
    }

    if (index >= size) {
        // Index out of bounds
        return false;
    }

    auto *valuePointer =
        rapidjson::Pointer(arrayPath.c_str()).Get(instance->document);
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

    instance->clearSettings(arrayPath + "/" + std::to_string(index) + "/");

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

    const auto &instance = SettingManager::getInstance();

    rapidjson::SizeType numValuesRemoved = 0;

    for (rapidjson::SizeType i = size - 1; i > 0; --i) {
        if (instance->_isNull(arrayPath + "/" + std::to_string(i))) {
            SettingManager::removeArrayValue(arrayPath, i);
            ++numValuesRemoved;
        }
    }

    return numValuesRemoved;
}

std::vector<std::string>
SettingManager::getObjectKeys(const std::string &objectPath)
{
    return SettingManager::getObjectKeys(objectPath,
                                         SettingManager::getInstance());
}

std::vector<std::string>
SettingManager::getObjectKeys(const std::string &objectPath,
                              std::shared_ptr<SettingManager> instance)
{
    std::vector<std::string> ret;

    auto *root = instance->get(objectPath.c_str());

    if (root == nullptr || !root->IsObject()) {
        return ret;
    }

    for (rapidjson::Value::ConstMemberIterator it = root->MemberBegin();
         it != root->MemberEnd(); ++it) {
        ret.emplace_back(it->name.GetString());
    }

    return ret;
}

void
SettingManager::clear()
{
    const auto &instance = SettingManager::getInstance();

    // Clear document
    rapidjson::Value(rapidjson::kObjectType).Swap(instance->document);

    // Clear map of settings
    std::lock_guard<std::mutex> lock(instance->settingsMutex);

    instance->settings.clear();
}

bool
SettingManager::gRemoveSetting(const std::string &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->_removeSetting(path);
}

bool
SettingManager::removeSetting(const std::string &path)
{
    return this->_removeSetting(path);
}

bool
SettingManager::_removeSetting(const std::string &path)
{
    auto ptr = rapidjson::Pointer(path.c_str());

    std::lock_guard<std::mutex> lock(this->settingsMutex);

    this->settings.erase(path);

    std::string pathWithExtendor;
    if (path.at(path.length() - 1) == '/') {
        pathWithExtendor = path;
    } else {
        pathWithExtendor = path + '/';
    }

    auto iter = this->settings.begin();
    auto endIter = this->settings.end();
    for (; iter != endIter;) {
        const auto &p = *iter;
        if (p.first.compare(0, pathWithExtendor.length(), pathWithExtendor) ==
            0) {
            rapidjson::Pointer(p.first.c_str()).Erase(this->document);
            this->settings.erase(iter++);
        } else {
            ++iter;
        }
    }

    return ptr.Erase(this->document);
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

void
SettingManager::setPath(const std::filesystem::path &newPath)
{
    this->filePath = newPath;
}

SettingManager::LoadError
SettingManager::gLoad(const std::filesystem::path &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->load(path);
}

SettingManager::LoadError
SettingManager::gLoadFrom(const std::filesystem::path &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->loadFrom(path);
}

SettingManager::LoadError
SettingManager::load(const std::filesystem::path &path,
                     std::optional<LoadOptions> overrideLoadOptions)
{
    if (!path.empty()) {
        this->filePath = path;
    }

    return this->loadFrom(this->filePath, overrideLoadOptions);
}

SettingManager::LoadError
SettingManager::loadFrom(const std::filesystem::path &path,
                         std::optional<LoadOptions> overrideLoadOptions)
{
    auto result = this->readFrom(path);

    auto options = overrideLoadOptions.value_or(this->loadOptions);

    if (result != LoadError::NoError) {
        if (options.attemptLoadFromTemporaryFile) {
            // Loading from initial settings file failed, attempt to load from temporary file
            auto tmpPath(path);
            tmpPath += ".tmp";

            auto tmpResult = this->readFrom(tmpPath);
            if (tmpResult == LoadError::NoError) {
                if (!this->writeTo(path)) {
                    return LoadError::SavingFromTemporaryFileFailed;
                }

                // The "settings.json.tmp" file was successfully read and saved to "settings.json"
                std::error_code ec;
                std::filesystem::remove(tmpPath, ec);
                if (ec) {
                    // The setting path is in a bad state,
                    // but we have successfully loaded from the .tmp file, and saved that content to settings.json.
                    //
                    // Next time the user saves, they will most likely get a save error because they couldn't save to
                    // the .tmp file. This is best handled there.
                    return LoadError::NoError;
                }
            }

            return tmpResult;
        }
    }

    return result;
}

SettingManager::SaveResult
SettingManager::gSave(const std::filesystem::path &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->save(path);
}

SettingManager::SaveResult
SettingManager::gSaveAs(const std::filesystem::path &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->saveAs(path);
}

SettingManager::SaveResult
SettingManager::save(const std::filesystem::path &path)
{
    if (!path.empty()) {
        this->filePath = path;
    }

    return this->saveAs(this->filePath);
}

SettingManager::SaveResult
SettingManager::saveAs(const std::filesystem::path &path)
{
    if (this->hasSaveMethodFlag(SaveMethod::OnlySaveIfChanged) &&
        !this->hasUnsavedChanges) {
        // No save necessary - no changes have been made
        return SaveResult::Skipped;
    }

    std::error_code ec;
    Backup::saveWithBackup(
        path, this->backup,
        [this](const auto &tmpPath, auto &ec) {
            if (!this->writeTo(tmpPath)) {
                ec = std::make_error_code(std::errc::io_error);
            } else {
                this->hasUnsavedChanges = false;
            }
        },
        ec);

    if (ec) {
        return SaveResult::Failed;
    }

    return SaveResult::Success;
}

bool
SettingManager::writeTo(const std::filesystem::path &path)
{
    std::ofstream fh(path.c_str(), std::ios::binary | std::ios::out);
    if (!fh) {
        // Unable to open file at `path`
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    this->document.Accept(writer);

    fh.write(buffer.GetString(), buffer.GetSize());

    return true;
}

SettingManager::LoadError
SettingManager::readFrom(const std::filesystem::path &_path)
{
    std::error_code ec;

    auto path = detail::RealPath(_path, ec);

    if (ec) {
        return LoadError::FileHandleError;
    }

    // Open file
    std::ifstream fh(path.c_str(), std::ios::binary | std::ios::in);
    if (!fh) {
        // Unable to open file at `path`
        return LoadError::CannotOpenFile;
    }

    // Read size of file
    auto fileSize = std::filesystem::file_size(path, ec);
    if (ec) {
        return LoadError::FileHandleError;
    }

    if (fileSize == 0) {
        // Nothing to load
        return LoadError::NoError;
    }

    // Create std::vector of appropriate size
    std::vector<char> fileBuffer;
    fileBuffer.resize(fileSize);

    // Read file data into buffer
    fh.read(&fileBuffer[0], fileSize);

    // Merge newly parsed config file into our pre-existing document
    // The pre-existing document might be empty, but we don't know that

    rapidjson::ParseResult ok = this->document.Parse(&fileBuffer[0], fileSize);

    // Make sure the file parsed okay
    if (!ok) {
        return LoadError::JSONParseError;
    }

    // This restricts config files a bit. They NEED to have an object root
    if (!this->document.IsObject()) {
        return LoadError::JSONParseError;
    }

    // Perform deep merge of objects
    // detail::mergeObjects(document, d, document.GetAllocator());

    this->notifyLoadedValues();

    return LoadError::NoError;
}

void
SettingManager::setBackupEnabled(bool enabled)
{
    this->backup.enabled = enabled;
}

void
SettingManager::setBackupSlots(uint8_t numSlots)
{
    this->backup.numSlots = numSlots;
}

const std::shared_ptr<SettingManager> &
SettingManager::getInstance()
{
    static auto m = std::make_shared<SettingManager>();

    return m;
}

std::weak_ptr<SettingData>
SettingManager::getSetting(const std::string &path,
                           std::shared_ptr<SettingManager> instance)
{
    if (!instance) {
        instance = SettingManager::getInstance();
    }

    std::lock_guard<std::mutex> lock(instance->settingsMutex);

    auto &setting = instance->settings[path];

    if (setting == nullptr) {
        // No setting has been created with this path
        setting.reset(new SettingData(path, instance));
    }

    return std::static_pointer_cast<SettingData>(setting);
}

std::shared_ptr<SettingData>
SettingManager::getSetting(const std::string &path)
{
    std::lock_guard<std::mutex> lock(this->settingsMutex);

    auto it = this->settings.find(path);

    if (it == this->settings.end()) {
        // no setting found at this path
        return nullptr;
    }

    return it->second;
}

}  // namespace pajlada::Settings
