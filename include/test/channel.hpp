#pragma once

#include "pajlada/settings/setting.hpp"

namespace pajlada {
namespace test {

class Channel
{
public:
    Channel(const std::string &_name)
        : name(_name)
        , localRoot(name, &globalRoot)
        , maxMessageLength("maxMessageLength", 240, &this->localRoot)
    {
    }

    std::string name;

    static settings::Setting<void> globalRoot;
    settings::Setting<void> localRoot;
    settings::Setting<int> maxMessageLength;
};

}  // namespace test
}  // namespace pajlada
