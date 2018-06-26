lcov -t "result" -o test.info -c -d . && genhtml --demangle-cpp -o res test.info
