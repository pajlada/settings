#pragma once

namespace pajlada {
namespace Settings {

// Custom "JSON Object {}" type
struct Object {
    bool
    operator==(const Object &)
    {
        return false;
    }
};

// Custom "JSON Array []" type
struct Array {
    bool
    operator==(const Array &)
    {
        return false;
    }
};

}  // namespace Settings
}  // namespace pajlada
