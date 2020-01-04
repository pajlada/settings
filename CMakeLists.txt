cmake_minimum_required(VERSION 3.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(settings)

if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake")
    message(STATUS "Using CONAN")
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

set(PajladaSettings_SOURCES
    src/settings/settingdata.cpp
    src/settings/settingmanager.cpp

    src/test/main.cpp

    src/test/map.cpp
    src/test/save.cpp
    src/test/static.cpp
    src/test/signal.cpp
    src/test/default.cpp
    src/test/scope.cpp
    src/test/remove.cpp
    src/test/serialize.cpp
    src/test/deserialize.cpp
    src/test/advanced-array.cpp
    src/test/bad-instance.cpp
    src/test/misc.cpp

    src/test/foo.cpp
    src/test/channel.cpp

    src/test/common.cpp
    )

include_directories(include)

add_definitions(-DPAJLADA_SETTINGS_DEBUG)
# add_definitions(-DPAJLADA_BOOST_ANY_SUPPORT)
# add_definitions(-DPAJLADA_SETTINGS_BOOST_FILESYSTEM)

add_executable(settings
    ${PajladaSettings_SOURCES}
    )

# Enable C++17
set_property(TARGET settings PROPERTY CXX_STANDARD 17)
set_property(TARGET settings PROPERTY CXX_STANDARD_REQUIRED ON)

option(USE_BOOST_FILESYSTEM "Use boost::filesystem instead of std filesystem" OFF)
if (USE_BOOST_FILESYSTEM)
    message(status "use boost")
    add_definitions(-DPAJLADA_SETTINGS_BOOST_FILESYSTEM)
endif()

if (DEFINED COVERAGE)
    message("Coverage flags enabled")
    target_compile_options(settings PRIVATE --coverage -fno-inline -O0 -g)
    target_link_libraries(settings PRIVATE --coverage)
endif()

# Dependencies
find_package(PajladaSignals REQUIRED)
include_directories(${PajladaSignals_INCLUDE_DIRS})

# break()

find_package(PajladaSerialize REQUIRED)
include_directories(${PajladaSerialize_INCLUDE_DIRS})

# rapidjson = c++ json library
if (MSVC)
    target_link_libraries(settings INTERFACE IMPORTED CONAN_PKG::rapidjson)
else()
    find_package(RapidJSON REQUIRED)
    include_directories(${RAPIDJSON_INCLUDE_DIRS})
endif()

# TODO: Try find_package, and if not found, do the conan link

set(USE_BOOST_COMPONENTS "")

if (MSVC)
    # TODO: Figure out right way to go about this for windows
else()
    get_directory_property( DirDefs COMPILE_DEFINITIONS )
    if (DirDefs MATCHES "PAJLADA_SETTINGS_BOOST_FILESYSTEM")
        set(USE_BOOST_COMPONENTS system filesystem)
    else()
        target_link_libraries(settings PRIVATE stdc++fs)
    endif()
endif()


if (MSVC)
    target_link_libraries(settings INTERFACE IMPORTED CONAN_PKG::Boost)
else()
    find_package(Boost REQUIRED ${USE_BOOST_COMPONENTS})
    include_directories(${Boost_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(settings LINK_PUBLIC ${Boost_LIBRARIES})
endif()

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

# Make a symlink for files
add_custom_command(
        OUTPUT symlink_generated_successfully
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_LIST_DIR}/files files
        )

add_custom_target(
        make_files_symlink
        DEPENDS symlink_generated_successfully)

add_dependencies(settings make_files_symlink)
