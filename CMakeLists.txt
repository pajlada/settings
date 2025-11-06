cmake_minimum_required(VERSION 3.15..4.1.1)

cmake_policy(SET CMP0091 NEW) # select MSVC runtime library through `CMAKE_MSVC_RUNTIME_LIBRARY`


list(APPEND CMAKE_MESSAGE_CONTEXT PajladaSettings)
project(PajladaSettings
    VERSION 0.3.0
    DESCRIPTION "Settings library"
    HOMEPAGE_URL "https://github.com/pajlada/settings"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

option(PAJLADA_SETTINGS_USE_CONAN "Use conan file manager to handle dependencies" OFF)
option(PAJLADA_SETTINGS_BUILD_TESTS "Build tests" OFF)

if (PAJLADA_SETTINGS_USE_CONAN)
    message(WARNING "The use of PAJLADA_SETTINGS_USE_CONAN is no longer supported, use conan by specifying the CMAKE_TOOLCHAIN_FILE parameter instead (e.g. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake)")
endif ()

set(PajladaSettings_SOURCES
    src/settings/backup.cpp
    src/settings/setting.cpp
    src/settings/settingdata.cpp
    src/settings/settingmanager.cpp

    src/settings/detail/rename.cpp
    src/settings/detail/realpath.cpp
    )

add_library(PajladaSettings STATIC ${PajladaSettings_SOURCES})
add_library(Pajlada::Settings ALIAS PajladaSettings)

# Enable strict compiler settings
if (MSVC)
    # Someone adds /W3 before we add /W4.
    # This makes sure, only /W4 is specified.
    string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /WX # treat warnings as errors
        )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wpedantic
        -Wextra
        -Werror
        )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wno-restrict
            )
    endif()
endif()

target_include_directories(PajladaSettings PUBLIC include)

# Dependencies
FetchContent_Declare(
    rjlib
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/external/rapidjson
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS NAMES RapidJSON
)
set(RAPIDJSON_BUILD_EXAMPLES Off CACHE INTERNAL "")
set(RAPIDJSON_BUILD_TESTS Off CACHE INTERNAL "")

FetchContent_Declare(
    PajladaSignals
    URL ${CMAKE_CURRENT_LIST_DIR}/external/signals
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    PajladaSerialize
    URL ${CMAKE_CURRENT_LIST_DIR}/external/serialize
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(rjlib PajladaSignals PajladaSerialize)

# Enable std::string overloads - this needs to be public because we use these overloads in headers
target_compile_definitions(PajladaSettings PUBLIC RAPIDJSON_HAS_STDSTRING=1)

target_link_libraries(PajladaSettings PUBLIC Pajlada::Signals)
target_link_libraries(PajladaSettings PUBLIC Pajlada::Serialize)

if(TARGET rapidjson)
    message(DEBUG "Linking to rapidjson target")
    target_link_libraries(PajladaSettings PUBLIC rapidjson)
elseif(DEFINED RapidJSON_SOURCE_DIR)
    message(DEBUG "RapidJSON_SOURCE_DIR defined, assuming this is a submodule/source build. (${RapidJSON_SOURCE_DIR})")
    target_include_directories(PajladaSettings SYSTEM PUBLIC ${RapidJSON_SOURCE_DIR}/include)
else()
    message(DEBUG "No rapidjson target found, this is most likely a system install. Adding include directories (${RAPIDJSON_INCLUDE_DIRS}) instead")
    target_include_directories(PajladaSettings SYSTEM PUBLIC ${RAPIDJSON_INCLUDE_DIRS})
endif()

if(PAJLADA_SETTINGS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
