#pragma once

#ifdef PAJLADA_SETTINGS_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
#else
#include <filesystem>
#endif

namespace pajlada {
namespace Settings {

#ifdef PAJLADA_SETTINGS_BOOST_FILESYSTEM
namespace fs = boost::filesystem;
using fs_error_code = boost::system::error_code;
#else
namespace fs = std::filesystem;
using fs_error_code = std::error_code;
#endif

}  // namespace Settings
}  // namespace pajlada

