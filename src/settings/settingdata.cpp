#include "pajlada/settings/settingdata.hpp"

namespace pajlada {
namespace settings {

ISettingData::ISettingData(const std::string &_key, Setting<Object> *_parent)
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



}  // namespace settings
}  // namespace pajlada
