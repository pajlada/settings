project(PajladaSettingsTest)

# For MSVC: Prevent overriding the parent project's compiler/linker settings
# See https://github.com/google/googletest/blob/main/googletest/README.md#visual-studio-dynamic-vs-static-runtimes
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../external/googletest" "googletest")

mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
    )

set_target_properties(gtest PROPERTIES FOLDER lib)
set_target_properties(gtest_main PROPERTIES FOLDER lib)
set_target_properties(gmock PROPERTIES FOLDER lib)
set_target_properties(gmock_main PROPERTIES FOLDER lib)

add_executable(${PROJECT_NAME}
    src/main.cpp

    src/map.cpp
    src/load.cpp
    src/save.cpp
    src/static.cpp
    src/signal.cpp
    src/default.cpp
    src/scope.cpp
    src/remove.cpp
    src/serialize.cpp
    src/deserialize.cpp
    src/advanced-array.cpp
    src/bad-instance.cpp
    src/misc.cpp
    src/listener.cpp
    src/backup.cpp
    src/multithread.cpp

    src/foo.cpp
    src/channel.cpp

    src/option-compare-before-set.cpp

    src/common.cpp
    )

if(PAJLADA_SETTINGS_COVERAGE)
    append_coverage_compiler_flags()
    message("++ ${PROJECT_NAME} Coverage enabled (${CMAKE_CXX_COMPILER_ID})")

    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest
        BASE_DIRECTORY "../"
        EXCLUDE "external/*" "tests/*"
        )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Pajlada::Settings)
target_link_libraries(${PROJECT_NAME} PRIVATE gtest)
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE PAJLADA_SETTINGS_DEBUG)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)


# Make a symlink for files
add_custom_command(
    OUTPUT symlink_generated_successfully
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_LIST_DIR}/files files
    )

add_custom_target(
    make_files_symlink
    DEPENDS symlink_generated_successfully)

add_dependencies(${PROJECT_NAME} make_files_symlink)
