#!/bin/sh

set -e

echo "hello!!!!!!!!! ${BUILD_TYPE}"

cat /etc/os-release

if [ "$BUILD_TYPE" = "submodule" ]; then
    git submodule update --init --recursive
fi

mkdir build && cd build

if [ "$BUILD_TYPE" = "conan" ]; then
    cmake -GNinja --log-level=DEBUG --log-context --debug-output --preset debug-conan ..
else
    cmake -GNinja --log-level=DEBUG --log-context --debug-output --preset debug ..
fi

cmake --build .

ctest
