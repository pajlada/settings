#!/bin/bash
lcov -o test.info.base -c -d . && \
lcov -r test.info.base '/usr/include/*' "$PWD/external/*" "$PWD/include/test*" "$PWD/src/test/*" -o test.info && \
genhtml --demangle-cpp -o res test.info
