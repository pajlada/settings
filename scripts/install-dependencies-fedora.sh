#!/bin/sh

echo "build type: ${BUILD_TYPE}"

if [ "$BUILD_TYPE" = "conan" ]; then
    # NOTE: Dependency state is not 100% conan here:
    #  rapidjson: conan
    #  PajladaSerialize: submodule
    #  PajladaSignals: submodule
    dnf install --assumeyes conan

    rm -rf external/rapidjson
elif [ "$BUILD_TYPE" = "system" ]; then
    #  rapidjson: system
    #  PajladaSerialize: system (installed through submodule)
    #  PajladaSignals: system (installed through submodule)
    dnf install --assumeyes rapidjson-devel

    # Install PajladaSerialize
    cmake -B build-pajlada-serialize external/serialize
    cmake --install build-pajlada-serialize

    # Install PajladaSignals
    cmake -B build-pajlada-signals external/signals
    cmake --install build-pajlada-signals

    rm -rf external
elif [ "$BUILD_TYPE" = "submodule" ]; then
    #  rapidjson: submodule
    #  PajladaSerialize: submodule
    #  PajladaSignals: submodule
    echo ""
fi
