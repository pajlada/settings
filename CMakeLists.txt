cmake_minimum_required(VERSION 3.15..4.1.1)
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY On) # For rapidjson

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

include(CMakePackageConfigHelpers)
include(FetchContent)
include(GNUInstallDirs)

option(PAJLADA_SETTINGS_USE_CONAN "Use conan file manager to handle dependencies" OFF)
option(PAJLADA_SETTINGS_BUILD_TESTS "Build tests" OFF)
option(PAJLADA_SETTINGS_SHARED_LIBS "Build pajlada-serialize as a shared library" ${PROJECT_IS_TOP_LEVEL})
option(PAJLADA_SETTINGS_INSTALL "Install pajlada-serialize" ${PROJECT_IS_TOP_LEVEL})

if (PAJLADA_SETTINGS_USE_CONAN)
    message(WARNING "The use of PAJLADA_SETTINGS_USE_CONAN is no longer supported, use conan by specifying the CMAKE_TOOLCHAIN_FILE parameter instead (e.g. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake)")
endif ()

if (PAJLADA_SETTINGS_SHARED_LIBS)
    set(BUILD_SHARED_LIBS OFF)
endif ()

# Dependencies
FetchContent_Declare(
    RapidJSON
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/external/rapidjson
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
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

FetchContent_MakeAvailable(RapidJSON PajladaSignals PajladaSerialize)

set(BUILD_SHARED_LIBS ${PAJLADA_SETTINGS_SHARED_LIBS})

add_library(PajladaSettings)
add_library(Pajlada::Settings ALIAS PajladaSettings)

set_target_properties(PajladaSettings PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "pajlada-settings"
    EXPORT_NAME Settings
)

# target_include_directories(PajladaSettings PUBLIC include)

add_subdirectory(src)
add_subdirectory(include)

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

if (PAJLADA_SETTINGS_INSTALL)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/PajladaSettingsConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMinorVersion
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PajladaSettingsConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/PajladaSettingsConfig.cmake
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/PajladaSettings"
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # export(EXPORT SettingsTargets
    #     FILE PajladaSettingsTargets.cmake
    #     NAMESPACE "Pajlada::"
    # )

    install(EXPORT SettingsTargets
        FILE PajladaSettingsTargets.cmake
        NAMESPACE "Pajlada::"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        COMPONENT PajladaSettings_Development
        EXPORT_LINK_INTERFACE_LIBRARIES
    )
endif ()

if(NOT PROJECT_IS_TOP_LEVEL)
    return(PROPAGATE
        PajladaSettings_VERSION
        PajladaSettings_VERSION_MAJOR
        PajladaSettings_VERSION_MINOR
        PajladaSettings_VERSION_PATCH
        PajladaSettings_VERSION_TWEAK
    )
endif()
