#include <pajlada/settings/settingdata.hpp>

namespace pajlada::Settings {

SettingData::SettingData(std::string _path,
                         std::weak_ptr<SettingManager> _instance)
    : path(std::move(_path))
    , instance(_instance)
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

rapidjson::Value *
SettingData::get() const
{
    auto locked = this->instance.lock();
    if (!locked) {
        return nullptr;
    }

    return locked->get(this->path.c_str());
}

}  // namespace pajlada::Settings
