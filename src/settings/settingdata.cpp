#include <pajlada/settings/settingdata.hpp>
#include <utility>

namespace pajlada::Settings {

SettingData::SettingData(std::string _path,
                         std::weak_ptr<SettingManager> _instance)
    : path(std::move(_path))
    , instance(std::move(_instance))
{
}

const std::string &
SettingData::getPath() const
{
    return this->path;
}

void
SettingData::notifyUpdate(const rapidjson::Value &value, SignalArgs args)
{
    ++this->updateIteration;

    this->updated.invoke(value, args);
}

int
SettingData::getUpdateIteration() const
{
    return this->updateIteration;
}

bool
SettingData::marshalJSON(const rapidjson::Value &v, SignalArgs &&args)
{
    auto locked = this->instance.lock();
    if (!locked) {
        return false;
    }
    return locked->set(path.c_str(), v, args);
}

bool
SettingData::unmarshalJSON(rapidjson::Document &doc)
{
    auto locked = this->instance.lock();
    if (!locked) {
        return false;
    }

    return locked->get(this->path.c_str(), doc);
}

}  // namespace pajlada::Settings
