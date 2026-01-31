cmake_minimum_required(VERSION 3.15...4.2.1)
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY On) # For rapidjson

project(PajladaSettingsSimple)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "out"
)

include(FetchContent)

FetchContent_Declare(
    RapidJSON
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)
set(RAPIDJSON_BUILD_EXAMPLES Off CACHE INTERNAL "")
set(RAPIDJSON_BUILD_TESTS Off CACHE INTERNAL "")

include(${CMAKE_CURRENT_LIST_DIR}/../IncludePajladaSettings.cmake)

FetchContent_MakeAvailable(RapidJSON PajladaSettings)

if(TARGET rapidjson)
    message(STATUS "Linking to rapidjson target")
    target_link_libraries(${PROJECT_NAME} PRIVATE rapidjson)
elseif(DEFINED RapidJSON_SOURCE_DIR)
    message(STATUS "RapidJSON_SOURCE_DIR defined, assuming this is a submodule/source build. (${RapidJSON_SOURCE_DIR})")
    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${RapidJSON_SOURCE_DIR}/include)
else()
    message(STATUS "No rapidjson target found, this is most likely a system install. Adding include directories (${RAPIDJSON_INCLUDE_DIRS}) instead")
    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
endif()

if (TARGET PajladaSettings)
    message(STATUS "FOUND PajladaSettings TARGET")
else()
    message(STATUS "No PajladaSettings target found")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE PajladaSettings)
