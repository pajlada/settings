#pragma once

#include <boost/any.hpp>

namespace pajlada {
namespace Settings {

struct SignalArgs {
    SignalArgs() = default;

    SignalArgs(boost::any &&_userData)
        : userData(std::move(_userData))
    {
    }

    enum Source {
        Unset,
        Setter,
        Unmarshal,
        OnConnect,
        External,
    } source = Source::Unset;

    boost::any userData;
    std::string path;
};

}  // namespace Settings
}  // namespace pajlada
