#include "pajlada/settings/jsonwrapper.hpp"
#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"

#include <string>

using namespace std;

namespace pajlada {
namespace settings {
namespace detail {

rapidjson::Value
JSONWrapper<Object>::create(const shared_ptr<SettingData<Object>> &)
{
    rapidjson::Value v(rapidjson::kObjectType);

    return v;
}

rapidjson::Value
JSONWrapper<Array>::create(const shared_ptr<SettingData<Array>> &)
{
    rapidjson::Value v(rapidjson::kArrayType);

    return v;
}

rapidjson::Value
JSONWrapper<int>::create(const shared_ptr<SettingData<int>> &setting)
{
    rapidjson::Value v;

    v.SetInt(setting->getValue());

    return v;
}

rapidjson::Value
JSONWrapper<bool>::create(const shared_ptr<SettingData<bool>> &setting)
{
    rapidjson::Value v;

    v.SetBool(setting->getValue());

    return v;
}

rapidjson::Value
JSONWrapper<float>::create(const shared_ptr<SettingData<float>> &setting)
{
    rapidjson::Value v;

    v.SetFloat(setting->getValue());

    return v;
}

rapidjson::Value
JSONWrapper<double>::create(const shared_ptr<SettingData<double>> &setting)
{
    rapidjson::Value v;

    v.SetDouble(setting->getValue());

    return v;
}

rapidjson::Value
JSONWrapper<string>::create(const shared_ptr<SettingData<string>> &setting)
{
    rapidjson::Value v;

    v.SetString(rapidjson::StringRef(setting->getValue().c_str()));

    return v;
}

}  // namespace detail
}  // namespace settings
}  // namespace pajlada
