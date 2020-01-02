#include <boost/filesystem.hpp>
#include <pajlada/settings/settingdata.hpp>

namespace fs = boost::filesystem;
using namespace std;

namespace pajlada {
namespace Settings {

SettingData::SettingData(fs::wpath _path, weak_ptr<SettingManager> _instance)
    : path(move(_path))
    , instance(_instance)
{
}

const fs::wpath &
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

}  // namespace Settings
}  // namespace pajlada
