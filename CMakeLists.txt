cmake_minimum_required(VERSION 3.15)

cmake_policy(SET CMP0091 NEW) # select MSVC runtime library through `CMAKE_MSVC_RUNTIME_LIBRARY`

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(PajladaSettings)

set(PAJLADA_SETTINGS_CXX_STANDARD "20")

option(PAJLADA_SETTINGS_USE_CONAN "Use conan file manager to handle dependencies" OFF)

if (PAJLADA_SETTINGS_USE_CONAN)
    message(WARNING "The use of PAJLADA_SETTINGS_USE_CONAN is no longer supported, use conan by specifying the CMAKE_TOOLCHAIN_FILE parameter instead (e.g. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake)")
endif ()

set(PajladaSettings_SOURCES
    src/settings/settingdata.cpp
    src/settings/settingmanager.cpp

    src/settings/detail/realpath.cpp
    )

add_library(PajladaSettings STATIC ${PajladaSettings_SOURCES})
add_library(Pajlada::Settings ALIAS PajladaSettings)

set_property(TARGET PajladaSettings PROPERTY CXX_STANDARD ${PAJLADA_SETTINGS_CXX_STANDARD})
set_property(TARGET PajladaSettings PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(PajladaSettings PUBLIC include)

option(PAJLADA_SETTINGS_USE_BOOST_FILESYSTEM "Use boost::filesystem instead of std filesystem" OFF)
if (PAJLADA_SETTINGS_USE_BOOST_FILESYSTEM)
    message(status "use boost")
    target_compile_definitions(PajladaSettings PUBLIC PAJLADA_SETTINGS_BOOST_FILESYSTEM)
endif()

if (DEFINED COVERAGE)
    message("++ Coverage flags enabled")
    target_compile_options(PajladaSettings PRIVATE --coverage -fno-inline -O0 -g)
    target_link_libraries(PajladaSettings PRIVATE --coverage)
endif()

if (PAJLADA_SETTINGS_BUILD_TESTS)
    message("++ Tests enabled")

    # For MSVC: Prevent overriding the parent project's compiler/linker settings
    # See https://github.com/google/googletest/blob/main/googletest/README.md#visual-studio-dynamic-vs-static-runtimes
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/external/googletest" "external/googletest")

    mark_as_advanced(
            BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
            gmock_build_tests gtest_build_samples gtest_build_tests
            gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
    )

    set_target_properties(gtest PROPERTIES FOLDER lib)
    set_target_properties(gtest_main PROPERTIES FOLDER lib)
    set_target_properties(gmock PROPERTIES FOLDER lib)
    set_target_properties(gmock_main PROPERTIES FOLDER lib)

    enable_testing()

    add_executable(settings-test
        src/test/main.cpp

        src/test/map.cpp
        src/test/load.cpp
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
        src/test/listener.cpp

        src/test/foo.cpp
        src/test/channel.cpp

        src/test/common.cpp
        )

    target_link_libraries(settings-test PRIVATE PajladaSettings gtest)

    add_test(AllTestsXD settings-test)

    target_compile_definitions(settings-test PRIVATE PAJLADA_SETTINGS_DEBUG)
    # target_compile_definitions(settings-test PRIVATE PAJLADA_BOOST_ANY_SUPPORT)
    # target_compile_definitions(settings-test PRIVATE PAJLADA_SETTINGS_BOOST_FILESYSTEM)

    set_property(TARGET settings-test PROPERTY CXX_STANDARD ${PAJLADA_SETTINGS_CXX_STANDARD})
    set_property(TARGET settings-test PROPERTY CXX_STANDARD_REQUIRED ON)

    # Enable strict compiler settings
    if (MSVC)
        # Someone adds /W3 before we add /W4.
        # This makes sure, only /W4 is specified.
        string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

        target_compile_options(settings-test PUBLIC
            /W4
            /WX # treat warnings as errors
        )
    else()
        target_compile_options(settings-test PUBLIC -Wall -Werror)
    endif()

    # Make a symlink for files
    add_custom_command(
            OUTPUT symlink_generated_successfully
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_LIST_DIR}/files files
            )

    add_custom_target(
            make_files_symlink
            DEPENDS symlink_generated_successfully)

        add_dependencies(settings-test make_files_symlink)
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

if (MSVC)
    # TODO: Figure out right way to go about this for windows
else()
    if (NOT PAJLADA_SETTINGS_USE_BOOST_FILESYSTEM)
		if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
			if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
				message(FATAL_ERROR "GCC version must be at least 8.0 if using std filesystem")
			endif()
		endif()
        target_link_libraries(PajladaSettings INTERFACE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
    endif()
endif()


if (PAJLADA_SETTINGS_USE_BOOST_FILESYSTEM)
    find_package(Boost REQUIRED system filesystem)
    target_link_libraries(PajladaSettings PUBLIC Boost::filesystem Boost::system)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
