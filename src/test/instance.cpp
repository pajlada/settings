#include <memory>
#include <pajlada/settings.hpp>
#include <pajlada/settings/instance-setting.hpp>
#include <pajlada/settings/instance.hpp>

#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

TEST_CASE("Instance", "[settings]")
{
    auto instance = std::make_shared<Instance>("files/in.instance.json");

    // INTERESTING USECASE: RETURN SHARED PTR INSTEAD
    // auto setting = instance->getSetting<std::string>("/foo/bar/baz");

    InstanceSetting<std::string> setting(instance, "/foo/bar/baz");

    setting.connect([](const std::string &xd) {
        std::cout << "Value updated to " << xd << '\n';
    });

    std::cout << "Setting value: " << setting.getValue() << '\n';

    setting.setValue("aaa");

    setting.setValue("bbb");

    // auto setting = instance.get("/foo/bar/baz");
}
