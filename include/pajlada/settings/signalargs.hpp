#pragma once

#ifndef PAJLADA_SETTINGS_NO_BOOST
#include <boost/any.hpp>
#endif

#include <string>

namespace pajlada {
namespace Settings {

struct SignalArgs {
    SignalArgs() = default;

#ifndef PAJLADA_SETTINGS_NO_BOOST
    SignalArgs(boost::any &&_userData)
        : userData(std::move(_userData))
    {
    }
#endif

    enum Source {
        Unset,
        Setter,
        Unmarshal,
        OnConnect,
        External,
    } source = Source::Unset;

#ifndef PAJLADA_SETTINGS_NO_BOOST
    boost::any userData;
#endif
    std::string path;
};

}  // namespace Settings
}  // namespace pajlada
