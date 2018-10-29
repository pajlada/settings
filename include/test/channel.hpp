#pragma once

#include <pajlada/settings/setting.hpp>

namespace pajlada {
namespace test {

class IndexedChannel
{
public:
    IndexedChannel(const unsigned &&_index);

    unsigned index;

    Settings::Setting<int> xDIndex;
    Settings::Setting<std::string> name;
};

class Channel
{
public:
    Channel(const std::string &_name);

    unsigned index;
    std::string name;

    Settings::Setting<int> maxMessageLength;
};

}  // namespace test
}  // namespace pajlada
