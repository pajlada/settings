#pragma once

#include "pajadog/setting.hpp"

namespace pajadog {

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

}  // namespace pajadog
