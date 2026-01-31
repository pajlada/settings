#!/bin/sh

set -e

echo "build type: ${BUILD_TYPE}"

cat /etc/os-release

if [ "$BUILD_TYPE" = "submodule" ]; then
    git submodule update --init --recursive
fi

if [ "$BUILD_TYPE" = "conan" ]; then
    cmake -GNinja --log-level=DEBUG --log-context --debug-output --preset debug-conan -B build
else
    cmake -GNinja --log-level=DEBUG --log-context --debug-output --preset debug -B build
fi

cmake --build build

ctest --test-dir build --output-on-failure
