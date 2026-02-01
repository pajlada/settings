#include <rapidjson/allocators.h>
#include <rapidjson/document.h>

#include <iostream>
#include <pajlada/settings.hpp>

using namespace pajlada::Settings;

int
main(int argc, char **argv)
{
    SettingManager sm;
    sm.saveMethod = SettingManager::SaveMethod::SaveManually;
    // int value = 5;
    // rapidjson::Document d;
    // auto middle = pajlada::Serialize<int>::get(value, d.GetAllocator());

    // bool error = false;
    // auto out = pajlada::Deserialize<int>::get(middle, &error);
    // std::cout << "test: " << value << " == " << out << '\n';
    std::cout << "test\n";
    return 0;
}
