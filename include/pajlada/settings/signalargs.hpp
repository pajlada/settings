#pragma once

#ifdef PAJLADA_BOOST_ANY_SUPPORT
#include <boost/any.hpp>
#endif

#include <string>

namespace pajlada::Settings {

struct SignalArgs {
    SignalArgs() = default;

    enum Source {
        Unset,
        Setter,
        Unmarshal,
        OnConnect,
        External,
    } source = Source::Unset;

    std::string path;

    bool writeToFile{true};
};

}  // namespace pajlada::Settings
