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
namespace errc = boost::system::errc;
namespace make_error_code_ns = boost::system::errc;
#else
namespace fs = std::filesystem;
using fs_error_code = std::error_code;
using errc = std::errc;
namespace make_error_code_ns = std;
#endif

}  // namespace Settings
}  // namespace pajlada

