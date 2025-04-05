cmake_minimum_required(VERSION 3.15)

cmake_policy(SET CMP0091 NEW) # select MSVC runtime library through `CMAKE_MSVC_RUNTIME_LIBRARY`

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(PajladaSettings
    VERSION 0.3.0
    DESCRIPTION "Settings library"
    HOMEPAGE_URL "https://github.com/pajlada/settings"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PAJLADA_SETTINGS_USE_CONAN "Use conan file manager to handle dependencies" OFF)
option(PAJLADA_SETTINGS_COVERAGE "Enable code coverage output" OFF)
option(PAJLADA_SETTINGS_BUILD_TESTS "Build tests" OFF)

if (PAJLADA_SETTINGS_USE_CONAN)
    message(WARNING "The use of PAJLADA_SETTINGS_USE_CONAN is no longer supported, use conan by specifying the CMAKE_TOOLCHAIN_FILE parameter instead (e.g. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake)")
endif ()

set(PajladaSettings_SOURCES
    src/settings/backup.cpp
    src/settings/settingdata.cpp
    src/settings/settingmanager.cpp

    src/settings/detail/rename.cpp
    src/settings/detail/realpath.cpp
    )

add_library(PajladaSettings STATIC ${PajladaSettings_SOURCES})
add_library(Pajlada::Settings ALIAS PajladaSettings)

if(PAJLADA_SETTINGS_COVERAGE)
    include(CodeCoverage)
    append_coverage_compiler_flags()
    message("++ ${PROJECT_NAME} Coverage enabled (${CMAKE_CXX_COMPILER_ID})")
endif()

set_property(TARGET PajladaSettings PROPERTY CXX_STANDARD 20)
set_property(TARGET PajladaSettings PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_definitions(${PROJECT_NAME} PUBLIC RAPIDJSON_HAS_STDSTRING)

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

if (DEFINED COVERAGE)
    message("++ Coverage flags enabled")
    target_compile_options(PajladaSettings PRIVATE --coverage -fno-inline -O0 -g)
    target_link_libraries(PajladaSettings PRIVATE --coverage)
endif()

# Dependencies

if (TARGET Pajlada::Signals)
    target_link_libraries(PajladaSettings PUBLIC Pajlada::Signals)
else()
    find_package(PajladaSignals REQUIRED)
    target_include_directories(PajladaSettings PUBLIC ${PajladaSignals_INCLUDE_DIRS})
endif()

if (TARGET Pajlada::Serialize)
    target_link_libraries(PajladaSettings PUBLIC Pajlada::Serialize)
else()
    find_package(PajladaSerialize REQUIRED)
    target_include_directories(PajladaSettings PUBLIC ${PajladaSerialize_INCLUDE_DIRS})
endif()

# rapidjson = c++ json library
if (TARGET RapidJSON::RapidJSON)
    message(STATUS "Using preexisting rapidjson target")
    target_link_libraries(PajladaSettings PUBLIC RapidJSON::RapidJSON)
else()
    find_package(RapidJSON REQUIRED)
    if (TARGET rapidjson)
        message(STATUS "Using rapidjson target")
        target_link_libraries(PajladaSettings PUBLIC rapidjson)
    else ()
        message(STATUS "Including rapidjson directory ${RAPIDJSON_INCLUDE_DIRS}")
        target_include_directories(PajladaSettings SYSTEM PUBLIC ${RAPIDJSON_INCLUDE_DIRS})
    endif ()
endif()

# TODO: Try find_package, and if not found, do the conan link

if(NOT MSVC)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
            message(FATAL_ERROR "GCC version must be at least 8.0 if using std filesystem")
        endif()
    endif()
    target_link_libraries(PajladaSettings INTERFACE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
endif()

if(PAJLADA_SETTINGS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
