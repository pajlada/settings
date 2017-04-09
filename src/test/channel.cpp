#include "test/channel.hpp"
#include "test/channelmanager.hpp"

namespace pajlada {
namespace test {

using namespace settings;

IndexedChannel::IndexedChannel(const unsigned &&_index,
                               settings::Setting<settings::Array> *parent)
    : index(_index)
    , localRoot(_index, parent)
    , xDIndex("index", _index, &this->localRoot)
    , name("name", "Name not loaded", &this->localRoot)
{
}

#ifndef ONLY_MINI_TEST
settings::Setting<settings::Object> Channel::globalRoot("channels");

Channel::Channel(const std::string &_name)
    : name(_name)
    , localRoot(name, &globalRoot)
    , maxMessageLength("maxMessageLength", 240, &this->localRoot)
{
}
#endif

}  // namespace test
}  // namespace pajlada
