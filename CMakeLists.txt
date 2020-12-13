cmake_minimum_required(VERSION 3.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(PajladaSettings)

if (MSVC)
    if (CONAN_EXPORTED)
        message(STATUS "Using CONAN")
        include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
        conan_basic_setup()
    else()
        # On Windows platform - use conan!
        include(conan.cmake)
        conan_cmake_run(CONANFILE conanfile.txt
                        BASIC_SETUP CMAKE_TARGETS
                        BUILD missing)
    endif()
endif()

set(PajladaSettings_SOURCES
    src/settings/settingdata.cpp
    src/settings/settingmanager.cpp

    src/settings/detail/realpath.cpp
    )

add_library(PajladaSettings OBJECT ${PajladaSettings_SOURCES})

set_property(TARGET PajladaSettings PROPERTY CXX_STANDARD 17)
set_property(TARGET PajladaSettings PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(PajladaSettings PUBLIC include)

option(USE_BOOST_FILESYSTEM "Use boost::filesystem instead of std filesystem" OFF)
if (USE_BOOST_FILESYSTEM)
    message(status "use boost")
    target_compile_definitions(PajladaSettings PUBLIC PAJLADA_SETTINGS_BOOST_FILESYSTEM)
endif()

if (DEFINED COVERAGE)
    message("++ Coverage flags enabled")
    target_compile_options(PajladaSettings PRIVATE --coverage -fno-inline -O0 -g)
    target_link_libraries(PajladaSettings PRIVATE --coverage)
endif()

if (BUILD_TESTS)
    message("++ Tests enabled")

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

        src/test/foo.cpp
        src/test/channel.cpp

        src/test/common.cpp
        )

    target_link_libraries(settings-test PRIVATE PajladaSettings)

    add_test(AllTestsXD settings-test)

    target_compile_definitions(settings-test PRIVATE PAJLADA_SETTINGS_DEBUG)
    # target_compile_definitions(settings-test PRIVATE PAJLADA_BOOST_ANY_SUPPORT)
    # target_compile_definitions(settings-test PRIVATE PAJLADA_SETTINGS_BOOST_FILESYSTEM)

    # Enable C++17
    set_property(TARGET settings-test PROPERTY CXX_STANDARD 17)
    set_property(TARGET settings-test PROPERTY CXX_STANDARD_REQUIRED ON)

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
find_package(PajladaSignals REQUIRED)
target_include_directories(PajladaSettings PUBLIC ${PajladaSignals_INCLUDE_DIRS})

find_package(PajladaSerialize REQUIRED)
target_include_directories(PajladaSettings PUBLIC ${PajladaSerialize_INCLUDE_DIRS})

# rapidjson = c++ json library
if (MSVC)
    target_link_libraries(PajladaSettings CONAN_PKG::rapidjson)
    target_link_libraries(PajladaSettings CONAN_PKG::booost)
else()
    find_package(RapidJSON REQUIRED)
endif()

# TODO: Try find_package, and if not found, do the conan link

if (MSVC)
    # TODO: Figure out right way to go about this for windows
else()
    if (NOT USE_BOOST_FILESYSTEM)
		if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
			if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
				message(FATAL_ERROR "GCC version must be at least 8.0 if using std filesystem")
			endif()
		endif()
        target_link_libraries(PajladaSettings INTERFACE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
    endif()

    find_package(RapidJSON REQUIRED)
    target_include_directories(PajladaSettings PUBLIC ${RAPIDJSON_INCLUDE_DIRS})
endif()


if (MSVC)
    # target_link_libraries(PajladaSettings INTERFACE IMPORTED CONAN_PKG::Boost)
    message(STATUS "Conan libs: ${CONAN_LIBS}")
    target_link_libraries(PajladaSettings PRIVATE ${CONAN_LIBS})
else()
    if (USE_BOOST_FILESYSTEM)
        find_package(Boost REQUIRED system filesystem)
        target_link_libraries(PajladaSettings PUBLIC Boost::filesystem Boost::system)
    endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
