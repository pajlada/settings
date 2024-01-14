#include "channel.hpp"

#include "channelmanager.hpp"

namespace pajlada::test {

using namespace Settings;

IndexedChannel::IndexedChannel(const unsigned _index)
    : index(_index)
    , xDIndex("/channelsArray/" + std::to_string(index) + "/index", _index)
    , name("/channelsArray/" + std::to_string(index) + "/name",
           "Name not loaded")
{
}

Channel::Channel(const std::string &_name)
    : name(_name)
    , maxMessageLength("/channels/" + this->name + "/maxMessageLength", 240)
{
}

}  // namespace pajlada::test
