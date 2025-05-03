#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>

#include <fstream>
#include <iostream>
#include <pajlada/settings/backup.hpp>
#include <pajlada/settings/detail/realpath.hpp>
#include <pajlada/settings/internal.hpp>
#include <pajlada/settings/settingdata.hpp>
#include <pajlada/settings/settingmanager.hpp>
#include <shared_mutex>
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

void
SettingManager::pp(const std::string &prefix)
{
    rapidjson::StringBuffer buffer;
    {
        std::shared_lock g(this->settingsDataMutex);
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        this->document.Accept(writer);
    }

    std::cout << prefix << buffer.GetString() << std::endl;
}

void
SettingManager::gPP(const std::string &prefix)
{
    auto instance = SettingManager::getInstance();

    instance->pp(prefix);
}

std::string
SettingManager::stringify(const rapidjson::Value &v)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);

    return {buffer.GetString()};
}

rapidjson::Value *
SettingManager::rawGet(const std::string &path)
{
    rapidjson::Pointer p(path);
    if (!p.IsValid()) {
        return nullptr;
    }
    return p.Get(this->document);
}

bool
SettingManager::get(const std::string &path, rapidjson::Document &doc)
{
    std::shared_lock g(this->settingsDataMutex);
    const auto *value = rapidjson::Pointer(path).Get(this->document);
    if (!value) {
        return false;
    }
    doc.CopyFrom(*value, doc.GetAllocator());
    return true;
}

bool
SettingManager::compare(const std::string &path, const rapidjson::Value &value)
{
    std::shared_lock g(this->settingsDataMutex);
    const auto *prevValue = rapidjson::Pointer(path).Get(this->document);
    return prevValue != nullptr && *prevValue == value;
}

bool
SettingManager::set(const std::string &path, const rapidjson::Value &value,
                    const SignalArgs &args)
{
    if (args.compareBeforeSet) {
        std::shared_lock g(this->settingsDataMutex);
        const auto *prevValue = rapidjson::Pointer(path).Get(this->document);
        if (prevValue != nullptr && *prevValue == value) {
            return false;
        }
    }

    this->hasUnsavedChanges = true;

    if (args.writeToFile) {
        {
            std::unique_lock g(this->settingsDataMutex);
            rapidjson::Pointer(path).Set(this->document, value);
        }

        if (this->hasSaveMethodFlag(SaveMethod::SaveOnSettingChange)) {
            this->save();
        }
    }

    this->notifyUpdate(path, value, args);

    return true;
}

void
SettingManager::notifyUpdate(const std::string &path,
                             const rapidjson::Value &value,
                             const SignalArgs &args)
{
    auto setting = this->getSetting(path);
    if (!setting) {
        return;
    }

    setting->notifyUpdate(value, args);
}

void
SettingManager::notifyLoadedValues()
{
    // Fill in any settings that registered before we called load
    this->settingsMutex.lock();

    auto loadedSettings = this->settings;

    this->settingsMutex.unlock();

    rapidjson::Document cpy;
    for (const auto &it : loadedSettings) {
        {
            std::shared_lock g(this->settingsDataMutex);
            auto *v = this->rawGet(it.first);
            if (v == nullptr) {
                continue;
            }
            cpy.CopyFrom(*v, cpy.GetAllocator());
        }

        // Maybe a "Load" source would make sense?
        SignalArgs args;
        args.source = SignalArgs::Source::Setter;

        it.second->notifyUpdate(cpy, args);
    }
}

rapidjson::SizeType
SettingManager::arraySize(const std::string &path)
{
    const auto &instance = SettingManager::getInstance();

    std::shared_lock g(instance->settingsDataMutex);
    auto *valuePointer = rapidjson::Pointer(path).Get(instance->document);
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
    std::shared_lock g(this->settingsDataMutex);
    auto *valuePointer = rapidjson::Pointer(path).Get(this->document);
    if (valuePointer == nullptr) {
        return true;
    }

    return valuePointer->IsNull();
}

void
SettingManager::setNull(const std::string &path)
{
    const auto &instance = SettingManager::getInstance();

    std::unique_lock g(instance->settingsDataMutex);
    rapidjson::Pointer(path).Set(instance->document, rapidjson::Value());
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

    {
        std::shared_lock g(instance->settingsDataMutex);
        auto *valuePointer =
            rapidjson::Pointer(arrayPath).Get(instance->document);
        if (valuePointer == nullptr) {
            return false;
        }

        rapidjson::Value &array = *valuePointer;

        if (index == size - 1) {
            // We want to remove the last element
            array.PopBack();
        } else {
            g.unlock();
            SettingManager::setNull(arrayPath + "/" + std::to_string(index));
        }
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
    auto instance = SettingManager::getInstance();

    std::vector<std::string> ret;

    std::shared_lock g(instance->settingsDataMutex);

    auto *root = instance->rawGet(objectPath);

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
    {
        std::unique_lock g(instance->settingsDataMutex);
        rapidjson::Value(rapidjson::kObjectType).Swap(instance->document);
    }

    // Clear map of settings
    std::lock_guard<std::mutex> lock(instance->settingsMutex);

    instance->settings.clear();
}

bool
SettingManager::removeSetting(const std::string &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->_removeSetting(path);
}

bool
SettingManager::_removeSetting(const std::string &path)
{
    auto ptr = rapidjson::Pointer(path.c_str());

    std::lock_guard<std::mutex> lock(this->settingsMutex);
    std::unique_lock g(this->settingsDataMutex);

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
SettingManager::load(const std::filesystem::path &path)
{
    if (!path.empty()) {
        this->filePath = path;
    }

    return this->loadFrom(this->filePath);
}

SettingManager::LoadError
SettingManager::loadFrom(const std::filesystem::path &_path)
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

    {
        std::unique_lock g(this->settingsDataMutex);
        rapidjson::ParseResult ok =
            this->document.Parse(&fileBuffer[0], fileSize);

        // Make sure the file parsed okay
        if (!ok) {
            return LoadError::JSONParseError;
        }

        // This restricts config files a bit. They NEED to have an object root
        if (!this->document.IsObject()) {
            return LoadError::JSONParseError;
        }
    }

    // Perform deep merge of objects
    // detail::mergeObjects(document, d, document.GetAllocator());

    this->notifyLoadedValues();

    return LoadError::NoError;
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
    {
        std::shared_lock g(this->settingsDataMutex);
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        this->document.Accept(writer);
    }

    fh.write(buffer.GetString(), buffer.GetSize());

    return true;
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
