#pragma once

#include "pajlada/settings/setting.hpp"

namespace pajlada {
namespace test {

class IndexedChannel
{
public:
    IndexedChannel(const unsigned &&_index,
                   settings::Setting<settings::Array> *parent);

    unsigned index;

    settings::Setting<settings::Object> localRoot;
    settings::Setting<int> xDIndex;
    settings::Setting<std::string> name;
};

#ifndef ONLY_MINI_TEST
class Channel
{
public:
    Channel(const std::string &_name);

    unsigned index;
    std::string name;

    static settings::Setting<settings::Object> globalRoot;
    settings::Setting<settings::Object> localRoot;
    settings::Setting<int> maxMessageLength;
};
#endif

}  // namespace test
}  // namespace pajlada
