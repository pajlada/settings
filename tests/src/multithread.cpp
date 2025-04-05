#include <gtest/gtest.h>

#include <string>
#include <thread>

#include "common.hpp"

using namespace pajlada::Settings;
using namespace pajlada;

// Note: This should be run with thread sanitizer

TEST(Multithread, basic)
{
    Setting<int> a("/multithread/basic/a");

    a.connect([](const int &it) { ASSERT_GE(it, 0); }, false);

    std::thread t1([&] {
        for (int i = 0; i < 1e4; i++) {
            a = i;
        }
    });
    std::thread t2([&] {
        int cur = 0;
        while (cur < 1e4 - 1) {
            cur = a;
            ASSERT_GE(cur, 0);
        }
    });

    t1.join();
    t2.join();
}

TEST(Multithread, string)
{
    Setting<std::string> a("/multithread/string/a");

    a.connect([](const std::string &it) { ASSERT_GE(it.find('='), 0); }, false);

    std::thread t1([&] {
        for (int i = 0; i <= 1e4; i++) {
            a = std::to_string(i) + '=';
        }
    });
    std::thread t2([&] {
        std::string cur;
        while (cur.length() < 6) {
            cur = a;
            ASSERT_GE(cur.find('='), 0);
        }
    });

    t1.join();
    t2.join();
}

TEST(Multithread, moreSettings)
{
    Setting<int> a("/multithread/basic/a");
    Setting<int> b("/multithread/basic/b");
    Setting<int> c("/multithread/basic/c");
    Setting<int> d("/multithread/basic/d");

    a.connect([](const int &it) { ASSERT_GE(it, 0); }, false);
    b.connect([](const int &it) { ASSERT_GE(it, 0); }, false);
    c.connect([](const int &it) { ASSERT_GE(it, 0); }, false);
    d.connect([](const int &it) { ASSERT_GE(it, 0); }, false);

    std::thread t1([&] {
        for (int i = 0; i <= 1e4; i++) {
            a = i;
        }
    });
    std::thread t2([&] {
        for (int i = 0; i <= 1e4; i++) {
            b = i;
        }
    });
    std::thread t3([&] {
        for (int i = 0; i <= 1e4; i++) {
            c = i;
        }
    });
    std::thread t4([&] {
        for (int i = 0; i <= 1e4; i++) {
            d = i;
        }
    });

    std::thread t5([&] {
        int cur = 0;
        while (cur < 4 * 1e4) {
            cur = a + b + c + d;
            ASSERT_GE(cur, 0);
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
}
