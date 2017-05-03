#include "test/channel.hpp"
#include "test/channelmanager.hpp"

namespace pajlada {
namespace test {

using namespace Settings;

IndexedChannel::IndexedChannel(const unsigned &&_index,
                               const Settings::Setting<Settings::Array> &parent)
    : index(_index)
    , localRoot(_index, parent)
    , xDIndex("index", this->localRoot, _index)
    , name("name", this->localRoot, "Name not loaded")
{
}

#ifndef ONLY_MINI_TEST
Settings::Setting<Settings::Object> Channel::globalRoot("channels");

Channel::Channel(const std::string &_name)
    : name(_name)
    , localRoot(name, globalRoot)
    , maxMessageLength("maxMessageLength", this->localRoot, 240)
{
}
#endif

}  // namespace test
}  // namespace pajlada
