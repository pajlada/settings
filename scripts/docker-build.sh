#!/usr/bin/env bash

_fedora_versions=("41" "42" "43" "44")
_build_types=("system" "conan" "submodule")

for _fedora_version in "${_fedora_versions[@]}"; do
    for _build_type in "${_build_types[@]}"; do
        docker build -f .docker/Dockerfile.fedora --tag "pajlada-settings:fedora-${_fedora_version}-${_build_type}" --build-arg BUILD_TYPE="${_build_type}" . &
    done
done

wait

echo "done"
