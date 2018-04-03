#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/setting.hpp"

#include <algorithm>
#include <string>

using namespace std;

namespace pajlada {
namespace Settings {

const string &
ISettingData::getPath() const
{
    return this->path;
}

bool
ISettingData::hasBeenSet() const
{
    return this->valueHasBeenSet;
}

void
ISettingData::marshal(rapidjson::Document &d)
{
    if (this->optionEnabled(SettingOption::DoNotWriteToJSON)) {
        PS_DEBUG("[" << this->path
                     << "] Skipping marshal due to `DoNotWriteToJSON` setting");
        return;
    }

    PS_DEBUG("[" << this->path << "] Marshalling into document");

    rapidjson::Value v = this->marshalInto(d);

    SettingManager::set(this->getPath().c_str(), std::move(v), d);
}

void
ISettingData::setPath(const string &_path)
{
    static string prefix = "/";

    // If _path begins with a /
    if (equal(begin(prefix), end(prefix), begin(_path))) {
        this->path = _path;
    } else {
        this->path = "/" + _path;
    }
}

rapidjson::Value *
ISettingData::get(rapidjson::Document &d)
{
    return SettingManager::get(this->path.c_str(), d);
}

}  // namespace Settings
}  // namespace pajlada
