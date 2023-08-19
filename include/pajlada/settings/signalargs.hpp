#pragma once

#ifdef PAJLADA_BOOST_ANY_SUPPORT
#include <boost/any.hpp>
#endif

#include <string>

namespace pajlada::Settings {

struct SignalArgs {
    SignalArgs() = default;

#ifdef PAJLADA_BOOST_ANY_SUPPORT
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

#ifdef PAJLADA_BOOST_ANY_SUPPORT
    boost::any userData;
#endif
    std::string path;

    bool writeToFile{true};
};

}  // namespace pajlada::Settings
