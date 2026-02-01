#!/usr/bin/env bash

set -o pipefail

_fedora_versions=("41" "42" "43" "44")
_build_types=("system")
# TECHNICALLY we only fetch rapidjson as a system dependency.
# serialize & signals are both still submodule builds
_example_import_methods=("SOURCE_DIR" "GIT" "FIND_PACKAGE")

for _fedora_version in "${_fedora_versions[@]}"; do
    for _build_type in "${_build_types[@]}"; do
        for _example_import_method in "${_example_import_methods[@]}"; do
            (
                _filename="logs/docker-example-${_fedora_version}-${_build_type}-${_example_import_method}.txt"
                echo "START docker example fedora ${_fedora_version} ${_build_type} $(date)" | tee -a "$_filename"
                if docker run --rm "pajlada-settings:fedora-${_fedora_version}-${_build_type}" ./scripts/run-example.sh "$_example_import_method" 2>&1 | tee -a "$_filename"; then
                    echo "DONE docker example fedora ${_fedora_version} ${_build_type} $(date)" | tee -a "$_filename"
                else
                    echo "ERROR docker example fedora ${_fedora_version} ${_build_type} $(date)" | tee -a "$_filename"
                fi
                )
        done
    done
done

wait

echo "done xd"
