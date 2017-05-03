#pragma once

#include "pajlada/settings/setting.hpp"

namespace pajlada {
namespace test {

class IndexedChannel
{
public:
    IndexedChannel(const unsigned &&_index,
                   const Settings::Setting<Settings::Array> &parent);

    unsigned index;

    Settings::Setting<Settings::Object> localRoot;
    Settings::Setting<int> xDIndex;
    Settings::Setting<std::string> name;
};

#ifndef ONLY_MINI_TEST
class Channel
{
public:
    Channel(const std::string &_name);

    unsigned index;
    std::string name;

    static Settings::Setting<Settings::Object> globalRoot;
    Settings::Setting<Settings::Object> localRoot;
    Settings::Setting<int> maxMessageLength;
};
#endif

}  // namespace test
}  // namespace pajlada
