#!/usr/bin/env bash

# _fedora_versions=("41" "42" "43" "44")
_build_types=("system" "conan" "submodule")
_fedora_versions=("41")

for _fedora_version in "${_fedora_versions[@]}"; do
    for _build_type in "${_build_types[@]}"; do
        (
            _filename="logs/docker-${_fedora_version}-${_build_type}.txt"
            echo "START docker test fedora ${_fedora_version} ${_build_type} $(date)" | tee -a "$_filename"
            docker run --rm "pajlada-settings:fedora-${_fedora_version}-${_build_type}" 2>&1 | tee -a "$_filename"
            echo "DONE docker test fedora ${_fedora_version} ${_build_type} $(date)" | tee -a "$_filename"
            ) &
    done
done

wait

echo "done xd"
