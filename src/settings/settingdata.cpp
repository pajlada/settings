#include <pajlada/settings/settingdata.hpp>

using namespace std;

namespace pajlada {
namespace Settings {

SettingData::SettingData(string _path, weak_ptr<SettingManager> _instance)
    : path(move(_path))
    , instance(_instance)
{
}

const string &
SettingData::getPath() const
{
    return this->path;
}

void
SettingData::notifyUpdate(const rapidjson::Value &value)
{
    ++this->updateIteration;

    // TODO: fill out signal args
    SignalArgs args;

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

}  // namespace Settings
}  // namespace pajlada
