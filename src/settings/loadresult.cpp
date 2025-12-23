#include <cassert>
#include <pajlada/settings/loadresult.hpp>

namespace pajlada::Settings {

LoadResult::~LoadResult()
{
#ifdef PAJLADA_SETTINGS_CHECKED_RESULT
    assert(this->checked && "Unchecked result");
#endif
}

}  // namespace pajlada::Settings
