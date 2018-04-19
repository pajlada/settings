#pragma once

#include "pajlada/settings/setting.hpp"

namespace pajlada {
namespace test {

class Foo
{
public:
    static Settings::Setting<int> i1;
    static Settings::Setting<int> i2;
    static Settings::Setting<int> i3;
    static Settings::Setting<std::string> s1;

    static Settings::Setting<float> f1;
    static Settings::Setting<float> f2;
    static Settings::Setting<float> f3;
    static Settings::Setting<float> f4;
    static Settings::Setting<float> f5;
    static Settings::Setting<float> f6;
    static Settings::Setting<float> f7;

    static Settings::Setting<double> d1;
    static Settings::Setting<double> d2;
    static Settings::Setting<double> d3;
    static Settings::Setting<double> d4;
    static Settings::Setting<double> d5;
    static Settings::Setting<double> d6;
    static Settings::Setting<double> d7;

    static Settings::Setting<bool> b1;
    static Settings::Setting<bool> b2;
    static Settings::Setting<bool> b3;
    static Settings::Setting<bool> b4;
    static Settings::Setting<bool> b5;
    static Settings::Setting<bool> b6;
    static Settings::Setting<bool> b7;

    static Settings::Setting<int> rootInt1;
    static Settings::Setting<int> rootInt2;
};

}  // namespace test
}  // namespace pajlada
