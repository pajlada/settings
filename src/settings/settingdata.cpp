#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/setting.hpp"

#include <algorithm>
#include <string>

using namespace std;

namespace pajlada {
namespace Settings {
namespace detail {

static atomic<uint64_t> latestConnectionID;

ISettingData::ISettingData()
    : connectionID(++latestConnectionID)
{
}

const string &
ISettingData::getPath() const
{
    return this->path;
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

void
ISettingData::setKey(const string &_key, const Setting<Object> &parent)
{
    auto data = parent.getData();

    this->path = data->getPath() + "/" + _key;
}

void
ISettingData::setIndex(unsigned _index, const Setting<Array> &parent)
{
    auto data = parent.getData();

    this->path = data->getPath() + "/" + to_string(_index);
}

}  // namespace detail
}  // namespace Settings
}  // namespace pajlada
