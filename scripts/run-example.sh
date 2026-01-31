#!/usr/bin/env bash

set -e

EXAMPLE_IMPORT_METHOD="$1"

echo "Example import type: ${EXAMPLE_IMPORT_METHOD}"

cat /etc/os-release

cmake -GNinja \
    --log-level=DEBUG \
    --log-context \
    --debug-output \
    --debug-find \
    --preset release \
    -B build \
    -S .

cmake --build build
sudo cmake --install build

_examples=("simple")
for _example in "${_examples[@]}"; do
    mkdir "examples/${_example}/build"
    cmake -GNinja \
        --log-level=DEBUG \
        --log-context \
        --debug-output \
        --debug-find \
        -DEXAMPLE_IMPORT_METHOD="${EXAMPLE_IMPORT_METHOD}" \
        -B "examples/${_example}/build" \
        -S "examples/${_example}"
    cmake --build "examples/${_example}/build"
    "./examples/${_example}/build/out"
done
