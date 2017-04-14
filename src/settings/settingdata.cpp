#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/setting.hpp"

#include <algorithm>
#include <string>

using namespace std;

namespace pajlada {
namespace settings {
namespace detail {

atomic<uint64_t> ISettingData::latestConnectionID = 0;

ISettingData::ISettingData(const string &_key, Setting<Object> *_parent)
    : key(_key)
    , index()
    , settingObjectParent(_parent)
    , settingArrayParent(nullptr)
    , connectionID(++this->latestConnectionID)
{
}

ISettingData::ISettingData(unsigned _index, Setting<Array> *_parent)
    : key()
    , index(_index)
    , settingObjectParent(nullptr)
    , settingArrayParent(_parent)
    , connectionID(++this->latestConnectionID)
{
}

ISettingData::ISettingData()
    : key()
    , index()
    , settingObjectParent()
    , settingArrayParent()
    , connectionID(++this->latestConnectionID)
{
}

const string &
ISettingData::getPath() const
{
    return this->path;
}

const string &
ISettingData::getKey() const
{
    return this->key;
}

unsigned
ISettingData::getIndex() const
{
    return this->index;
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
    this->key = _key;

    auto data = parent.getData();

    this->path = data->getPath() + "/" + _key;
}

void
ISettingData::setIndex(unsigned _index, const Setting<Array> &parent)
{
    this->index = _index;

    auto data = parent.getData();

    this->path = data->getPath() + "/" + to_string(_index);
}

}  // namespace detail
}  // namespace settings
}  // namespace pajlada
