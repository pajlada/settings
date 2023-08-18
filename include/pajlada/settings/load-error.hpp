#pragma once

namespace pajlada {
namespace Settings {

enum class LoadError {
    NoError,
    CannotOpenFile,
    FileHandleError,
    FileReadError,
    FileSeekError,
    JSONParseError,
};

template <typename Enumeration>
auto
as_integer(Enumeration const value) ->
    typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

}  // namespace Settings
}  // namespace pajlada
