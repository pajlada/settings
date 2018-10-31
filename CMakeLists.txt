cmake_minimum_required(VERSION 3.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(settings)

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
add_definitions(-DPAJLADA_BOOST_ANY_SUPPORT)
# add_definitions(-DPAJLADA_SETTINGS_ENABLE_EXCEPTIONS)

add_executable(settings
    ${PajladaSettings_SOURCES}
    )

# Enable C++17
set_property(TARGET settings PROPERTY CXX_STANDARD 17)
set_property(TARGET settings PROPERTY CXX_STANDARD_REQUIRED ON)


# Dependencies
find_package(PajladaSignals REQUIRED)
include_directories(${PajladaSignals_INCLUDE_DIRS})

find_package(PajladaSerialize REQUIRED)
include_directories(${PajladaSerialize_INCLUDE_DIRS})

# rapidjson = c++ json library
find_package(RapidJSON REQUIRED)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

find_package(Boost REQUIRED )
include_directories(${Boost_INCLUDE_DIRS})

if (MSVC)

else()
    # list(APPEND CMAKE_CXX_FLAGS "-Wall -Wno-unused-function -Wno-switch")


    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--dynamic-list-data")
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
