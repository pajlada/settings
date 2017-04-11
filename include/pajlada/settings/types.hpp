#pragma once

#include <ostream>

namespace pajlada {
namespace settings {

// Custom "JSON Object {}" type
struct Object {
};

// Custom "JSON Array []" type
struct Array {
};

static std::ostream &
operator<<(std::ostream &os, const Object &)
{
    return os << "{}";
}

static std::ostream &
operator<<(std::ostream &os, const Array &)
{
    return os << "[]";
}

}  // namespace settings
}  // namespace pajlada
