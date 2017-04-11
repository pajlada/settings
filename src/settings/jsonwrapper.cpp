#include "pajlada/settings/jsonwrapper.hpp"
#include "pajlada/settings/settingdata.hpp"
#include "pajlada/settings/settingmanager.hpp"

namespace pajlada {
namespace settings {

rapidjson::Value
JSONWrapper<Object>::create(const std::shared_ptr<SettingData<Object>> &setting)
{
    rapidjson::Value v;

    v.SetObject();

    std::cout << "Create object: " << setting->getKey().c_str() << std::endl;

    return v;
}

rapidjson::Value
JSONWrapper<Array>::create(const std::shared_ptr<SettingData<Array>> &setting)
{
    rapidjson::Value v;

    v.SetArray();

    std::cout << "Create array: " << setting->getKey().c_str() << std::endl;

    return v;
}

rapidjson::Value
JSONWrapper<int>::create(const std::shared_ptr<SettingData<int>> &setting)
{
    rapidjson::Value v;
    v.SetInt(setting->getValue());
    return v;
}

rapidjson::Value
JSONWrapper<bool>::create(const std::shared_ptr<SettingData<bool>> &setting)
{
    rapidjson::Value v;
    v.SetBool(setting->getValue());
    return v;
}

rapidjson::Value
JSONWrapper<float>::create(const std::shared_ptr<SettingData<float>> &setting)
{
    rapidjson::Value v;
    v.SetFloat(setting->getValue());
    return v;
}

rapidjson::Value
JSONWrapper<double>::create(const std::shared_ptr<SettingData<double>> &setting)
{
    rapidjson::Value v;
    v.SetDouble(setting->getValue());
    return v;
}

rapidjson::Value
JSONWrapper<std::string>::create(
    const std::shared_ptr<SettingData<std::string>> &setting)
{
    rapidjson::Value v;
    v.SetString(rapidjson::StringRef(setting->getValue().c_str()));
    return v;
}

}  // namespace settings
}  // namespace pajlada