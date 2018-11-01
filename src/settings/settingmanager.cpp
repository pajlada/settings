#include <pajlada/settings/settingmanager.hpp>

#include <pajlada/settings/settingdata.hpp>

#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>

#include <iostream>
#include <string>

#ifdef PAJLADA_SETTINGS_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
using fs_error_code = boost::system::error_code;
#else
#include <filesystem>
namespace fs = std::filesystem;
using fs_error_code = std::error_code;
#endif

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
    if (this->hasSaveMethodFlag(SaveMethod::SaveOnExit)) {
        this->save();
    }
}

void
SettingManager::pp(const string &prefix)
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    this->document.Accept(writer);

    cout << prefix << buffer.GetString() << endl;
}

void
SettingManager::gPP(const string &prefix)
{
    auto instance = SettingManager::getInstance();

    instance->pp(prefix);
}

string
SettingManager::stringify(const rapidjson::Value &v)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);

    return string(buffer.GetString());
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
    rapidjson::Pointer(path).Set(this->document, value);

    if (this->hasSaveMethodFlag(SaveMethod::SaveOnSettingChange)) {
        this->save();
    }

    this->notifyUpdate(path, value, std::move(args));

    return true;
}

void
SettingManager::notifyUpdate(const string &path, const rapidjson::Value &value,
                             SignalArgs args)
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
SettingManager::arraySize(const string &path)
{
    const auto &instance = SettingManager::getInstance();

    auto valuePointer =
        rapidjson::Pointer(path.c_str()).Get(instance->document);
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
SettingManager::isNull(const string &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->_isNull(path);
}

bool
SettingManager::_isNull(const string &path)
{
    auto valuePointer = rapidjson::Pointer(path.c_str()).Get(this->document);
    if (valuePointer == nullptr) {
        return true;
    }

    return valuePointer->IsNull();
}

void
SettingManager::setNull(const string &path)
{
    const auto &instance = SettingManager::getInstance();

    rapidjson::Pointer(path.c_str())
        .Set(instance->document, rapidjson::Value());
}

bool
SettingManager::removeArrayValue(const string &arrayPath,
                                 rapidjson::SizeType index)
{
    const auto &instance = SettingManager::getInstance();

    instance->clearSettings(arrayPath + "/" + to_string(index) + "/");

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
        rapidjson::Pointer(arrayPath.c_str()).Get(instance->document);
    if (valuePointer == nullptr) {
        return false;
    }

    rapidjson::Value &array = *valuePointer;

    if (index == size - 1) {
        // We want to remove the last element
        array.PopBack();
    } else {
        SettingManager::setNull(arrayPath + "/" + to_string(index));
    }

    instance->clearSettings(arrayPath + "/" + to_string(index) + "/");

    return true;
}

rapidjson::SizeType
SettingManager::cleanArray(const string &arrayPath)
{
    rapidjson::SizeType size = SettingManager::arraySize(arrayPath);

    if (size == 0) {
        // No values to remove
        return 0;
    }

    const auto &instance = SettingManager::getInstance();

    rapidjson::SizeType numValuesRemoved = 0;

    for (rapidjson::SizeType i = size - 1; i > 0; --i) {
        if (instance->_isNull(arrayPath + "/" + to_string(i))) {
            SettingManager::removeArrayValue(arrayPath, i);
            ++numValuesRemoved;
        }
    }

    return numValuesRemoved;
}

vector<string>
SettingManager::getObjectKeys(const string &objectPath)
{
    auto instance = SettingManager::getInstance();

    vector<string> ret;

    auto root = instance->get(objectPath.c_str());

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
    lock_guard<mutex> lock(instance->settingsMutex);

    instance->settings.clear();
}

bool
SettingManager::removeSetting(const string &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->_removeSetting(path);
}

bool
SettingManager::_removeSetting(const string &path)
{
    auto ptr = rapidjson::Pointer(path.c_str());

    lock_guard<mutex> lock(this->settingsMutex);

    this->settings.erase(path);

    string pathWithExtendor;
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
SettingManager::clearSettings(const string &root)
{
    lock_guard<mutex> lock(this->settingsMutex);

    vector<string> keysToBeRemoved;

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
SettingManager::setPath(const std::string &newPath)
{
    this->filePath = newPath;
}

SettingManager::LoadError
SettingManager::gLoad(const std::string &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->load(path);
}

SettingManager::LoadError
SettingManager::gLoadFrom(const std::string &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->loadFrom(path);
}

SettingManager::LoadError
SettingManager::load(const std::string &path)
{
    if (!path.empty()) {
        this->filePath = path;
    }

    return this->loadFrom(this->filePath);
}

SettingManager::LoadError
SettingManager::loadFrom(const std::string &path)
{
    // Open file
    FILE *fh = fopen(path.c_str(), "rb");
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
    unique_ptr<char[]> fileBuffer(new char[fileSize]);

    // Read file data into buffer
    auto readBytes = fread(fileBuffer.get(), 1, fileSize, fh);

    if (readBytes != static_cast<size_t>(fileSize)) {
        // Error reading the buffer
        fclose(fh);

        return LoadError::FileReadError;
    }

    // Close file
    fclose(fh);

    // XXX: Temporarily don't delete the buffer
    // delete[] fileBuffer;

    // Merge newly parsed config file into our pre-existing document
    // The pre-existing document might be empty, but we don't know that

    rapidjson::ParseResult ok =
        this->document.Parse(fileBuffer.get(), fileSize);

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

bool
SettingManager::gSave(const std::string &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->save(path);
}

bool
SettingManager::gSaveAs(const std::string &path)
{
    const auto &instance = SettingManager::getInstance();

    return instance->saveAs(path);
}

bool
SettingManager::save(const std::string &path)
{
    if (!path.empty()) {
        this->filePath = path;
    }

    return this->saveAs(this->filePath);
}

bool
SettingManager::saveAs(const std::string &path)
{
    auto res = this->_save(path + ".tmp");
    if (!res) {
        return res;
    }

    fs_error_code ec;

    if (this->backup.enabled) {
        if (this->backup.slots > 1) {
            // Remove top slot backup
            fs::remove(path + ".bkp-" + std::to_string(this->backup.slots), ec);

            // Shift backups one slot up
            for (uint8_t slotIndex = this->backup.slots - 1; slotIndex >= 1;
                 --slotIndex) {
                auto p1 = path + ".bkp-" + std::to_string(slotIndex);
                auto p2 = path + ".bkp-" + std::to_string(slotIndex + 1);
                fs::rename(p1, p2, ec);
            }
        }

        // Move current save to first backup slot
        fs::rename(path, path + ".bkp-1", ec);
    }

    fs::rename(path + ".tmp", path, ec);

    if (ec) {
        return false;
    }

    return true;
}
bool
SettingManager::_save(const std::string &path)
{
    FILE *fh = fopen(path.c_str(), "wb+");
    if (fh == nullptr) {
        // Unable to open file at `path`
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    this->document.Accept(writer);

    auto writtenBytes = fwrite(buffer.GetString(), 1, buffer.GetSize(), fh);

    // Close file handle
    fclose(fh);

    return writtenBytes == buffer.GetSize();
}

void
SettingManager::setBackupEnabled(bool enabled)
{
    this->backup.enabled = enabled;
}

void
SettingManager::setBackupSlots(uint8_t numSlots)
{
    this->backup.slots = numSlots;
}

weak_ptr<SettingData>
SettingManager::getSetting(const string &path,
                           shared_ptr<SettingManager> instance)
{
    if (!instance) {
        instance = SettingManager::getInstance();
    }

    lock_guard<mutex> lock(instance->settingsMutex);

    auto &setting = instance->settings[path];

    if (setting == nullptr) {
        // No setting has been created with this path
        setting.reset(new SettingData(path, instance));
    }

    return static_pointer_cast<SettingData>(setting);
}

shared_ptr<SettingData>
SettingManager::getSetting(const string &path)
{
    lock_guard<mutex> lock(this->settingsMutex);

    auto it = this->settings.find(path);

    if (it == this->settings.end()) {
        // no setting found at this path
        return nullptr;
    }

    return it->second;
}

}  // namespace Settings
}  // namespace pajlada
