#include "channel.hpp"

#include "channelmanager.hpp"

namespace pajlada::test {

using namespace Settings;

IndexedChannel::IndexedChannel(const unsigned _index,
                               const std::shared_ptr<SettingManager> &sm)
    : index(_index)
    , xDIndex("/channelsArray/" + std::to_string(index) + "/index", _index, sm)
    , name("/channelsArray/" + std::to_string(index) + "/name",
           "Name not loaded", sm)
{
}

Channel::Channel(const std::string &_name,
                 const std::shared_ptr<SettingManager> &sm)
    : name(_name)
    , maxMessageLength("/channels/" + this->name + "/maxMessageLength", 240, sm)
{
}

}  // namespace pajlada::test
