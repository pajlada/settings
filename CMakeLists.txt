cmake_minimum_required(VERSION 3.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(settings)

set(PajladaSettings_SOURCES
    src/main.cpp
    src/test/foo.cpp
    src/test/channel.cpp
    src/settings/settingdata.cpp
    src/settings/settingmanager.cpp
    src/testhelpers.cpp
    src/test-remove.cpp
    src/test-exceptions.cpp
    src/test-serialize.cpp
    src/test-deserialize.cpp
    )

include_directories(include)

add_definitions(-DPAJLADA_SETTINGS_DEBUG)
add_definitions(-DPAJLADA_SETTINGS_ENABLE_EXCEPTIONS)

add_executable(settings
    ${PajladaSettings_SOURCES}
    )

# Enable C++14
set_property(TARGET settings PROPERTY CXX_STANDARD 14)

# Dependencies
find_package(PajladaSignals REQUIRED)
include_directories(${PajladaSignals_INCLUDE_DIRS})

if (MSVC)

else()
    list(APPEND CMAKE_CXX_FLAGS "-Wall -Wno-unused-function -Wno-switch")
endif()

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
