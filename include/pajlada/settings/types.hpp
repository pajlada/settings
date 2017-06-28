#pragma once

namespace pajlada {
namespace Settings {

// Custom "JSON Object {}" type
struct Object {
    bool operator==(const Object &rhs)
    {
        return false;
    }
};

// Custom "JSON Array []" type
struct Array {
    bool operator==(const Array &rhs)
    {
        return false;
    }
};

}  // namespace Settings
}  // namespace pajlada
