These scripts are purely for development. They are not necessary for packaging or building.

Run `./scripts/docker-build.sh` to build `4*3` docker images under `pajlada-settings:` that contain your source dir.

Run them with `./scripts/docker-test.sh` and they will all build with system dependencies, submodules, or conan to ensure things build & test as expected.

There's no error reporting, but `rg -i 'error' logs/*.txt` is good about reporting things.
