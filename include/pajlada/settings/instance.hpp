#pragma once

#include <rapidjson/pointer.h>

#include <algorithm>
#include <any>
#include <cinttypes>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <pajlada/settings/common.hpp>
#include <pajlada/settings/detail/fs.hpp>
#include <pajlada/settings/detail/load-object.hpp>
#include <pajlada/settings/detail/realpath.hpp>
#include <pajlada/settings/load-error.hpp>
#include <pajlada/settings/signalargs.hpp>
#include <pajlada/signals.hpp>
#include <shared_mutex>
#include <vector>

namespace pajlada {
namespace Settings {

class SettingData;

/**
 * A SettingInstance governs over a single setting file
 **/
class Instance
{
public:
    Instance(const fs::path &_path)
        : path(_path)
    {
        auto error = this->load();
        std::cout << "Load error: " << as_integer(error) << "\n";
    }

    LoadError
    load()
    {
        fs_error_code ec;

        auto realPath = detail::RealPath(this->path, ec);

        if (ec) {
            return LoadError::FileHandleError;
        }

        // Open file
        std::ifstream fh(realPath.c_str(), std::ios::binary | std::ios::in);
        if (!fh) {
            // Unable to open file at `path`
            return LoadError::CannotOpenFile;
        }

        // Read size of file
        auto fileSize = fs::file_size(realPath, ec);
        if (ec) {
            return LoadError::FileHandleError;
        }

        if (fileSize == 0) {
            // Nothing to load
            return LoadError::NoError;
        }

        // Create vector of appropriate size
        std::vector<char> fileBuffer;
        fileBuffer.resize(fileSize);

        // Read file data into buffer
        fh.read(&fileBuffer[0], fileSize);

        // Merge newly parsed config file into our pre-existing document
        // The pre-existing document might be empty, but we don't know that

        rapidjson::Document document;

        rapidjson::ParseResult ok = document.Parse(&fileBuffer[0], fileSize);

        // Make sure the file parsed okay
        if (!ok) {
            return LoadError::JSONParseError;
        }

        // This restricts config files a bit. They NEED to have an object root
        if (!document.IsObject()) {
            return LoadError::JSONParseError;
        }

        detail::loadObject(document, "", this->data);

        // TODO: Should we notify subscribers?

        for (const auto &p : this->data) {
            std::cout << p.first << ": " << p.second.type().name() << '\n';
        }

        return LoadError::NoError;
    }

    Instance(const Instance &other) = delete;
    Instance(Instance &&other) = delete;

    Instance &operator=(const Instance &other) = delete;
    Instance &operator=(Instance &&other) = delete;

    ~Instance()
    {
        if (this->hasSaveMethodFlag(SaveMethod::SaveOnExit)) {
            this->save();
        }
    }

    void
    save()
    {
        rapidjson::Document document;

        for (const auto &p : this->data) {
            auto jsonValue =
                Serialize<std::any>::get(p.second, document.GetAllocator());
            // TODO: Type erasure to call the correct serializer?
            rapidjson::Pointer(p.first.c_str()).Set(document, jsonValue);
        }

        // TODO: Save to file
    }

    using ValueUpdatedCallback = std::function<void(const std::any &)>;

    mutable std::shared_mutex valueUpdatedMutex;
    std::map<std::string, pajlada::Signals::Signal<const std::any &>>
        valueUpdatedCallbacks;

    std::unique_ptr<Signals::ScopedConnection>
    registerOnValueUpdated(const std::string &settingPath,
                           ValueUpdatedCallback cb)
    {
        std::unique_lock lock(this->valueUpdatedMutex);

        auto &signal = this->valueUpdatedCallbacks[settingPath];

        return std::make_unique<Signals::ScopedConnection>(signal.connect(cb));

        // this->valueUpdatedCallbacks.insert({settingPath, cb});
    }

    void
    callOnValueUpdated(const std::string &settingPath, const std::any &newValue)
    {
        std::shared_lock lock(this->valueUpdatedMutex);

        auto it = this->valueUpdatedCallbacks.find(settingPath);

        if (it == this->valueUpdatedCallbacks.end()) {
            // No listeners on this path
            return;
        }

        pajlada::Signals::Signal<const std::any &> &signal = it->second;

        signal.invoke(newValue);
    }

    mutable std::shared_mutex dataMutex;
    std::map<std::string, std::any> data;

    void
    set(const std::string &settingPath, std::any settingValue)
    {
        {
            std::unique_lock lock(this->dataMutex);

            this->data[settingPath] = settingValue;
        }
        std::cout << "Instance::set:" << settingValue.type().name() << "\n";

        this->callOnValueUpdated(settingPath, settingValue);
    }

    template <class Type>
    OptionalType<Type>
    get(const std::string &settingPath)
    {
        std::shared_lock lock(this->dataMutex);

        auto settingValueIt = this->data.find(settingPath);

        if (settingValueIt == this->data.end()) {
            return OptionalNull;
        }

        const std::any &v = settingValueIt->second;

        return std::any_cast<Type>(v);
    }

    // Print given document json data prettily
    void pp(const std::string &prefix = std::string());
    static std::string stringify(const rapidjson::Value &v);

    rapidjson::Value *get(const char *path);
    bool set(const char *path, const rapidjson::Value &value,
             SignalArgs args = SignalArgs());

private:
    // Called from set
    void notifyUpdate(const std::string &path, const rapidjson::Value &value,
                      SignalArgs args = SignalArgs());

    // Called from load
    void notifyLoadedValues();

public:
    // Useful array helper methods
    static rapidjson::SizeType arraySize(const std::string &path);
    static bool isNull(const std::string &path);
    bool _isNull(const std::string &path);
    static void setNull(const std::string &path);

    // Basically the same as setNull, except we fully remove a value if it's the
    // last index of the array
    static bool removeArrayValue(const std::string &arrayPath,
                                 rapidjson::SizeType index);

    static rapidjson::SizeType cleanArray(const std::string &arrayPath);

    // Useful object helper methods
    static std::vector<std::string> getObjectKeys(
        const std::string &objectPath);

    static void clear();

    //static std::weak_ptr<SettingData> getSetting(
    //    const std::string &path, std::shared_ptr<SettingManager> instance);

    static bool removeSetting(const std::string &path);

public:
    enum class SaveMethod : uint64_t {
        SaveOnExit = (1ull << 1ull),
        SaveOnSettingChange = (1ull << 2ull),

        // Force user to manually call SettingsManager::save() to save
        SaveManually = 0,
        SaveAllTheTime = SaveOnExit | SaveOnSettingChange,
    } saveMethod = SaveMethod::SaveOnExit;

    // Returns true if the given save method is activated
    inline bool
    hasSaveMethodFlag(SaveMethod testSaveMethod) const
    {
        return (static_cast<uint64_t>(this->saveMethod) &
                static_cast<uint64_t>(testSaveMethod)) != 0;
    }

private:
    struct {
        bool enabled{};
        uint8_t numSlots = 3;
    } backup;

public:
    void setBackupEnabled(bool enabled = true);
    void setBackupSlots(uint8_t numSlots);

private:
    const fs::path path;

    std::mutex settingsMutex;

    //       path         setting
    std::map<std::string, std::shared_ptr<SettingData>> settings;
};

}  // namespace Settings
}  // namespace pajlada
