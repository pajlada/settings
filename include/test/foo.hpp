#pragma once

#include "pajadog/setting.hpp"

namespace pajadog {

class Foo
{
public:
    static settings::Setting<int> i1;
    static settings::Setting<int> i2;
    static settings::Setting<int> i3;
    static settings::Setting<std::string> s1;

    static settings::Setting<float> f1;
    static settings::Setting<float> f2;
    static settings::Setting<float> f3;
    static settings::Setting<float> f4;
    static settings::Setting<float> f5;
    static settings::Setting<float> f6;

    static settings::Setting<double> d1;
    static settings::Setting<double> d2;
    static settings::Setting<double> d3;
    static settings::Setting<double> d4;
    static settings::Setting<double> d5;
    static settings::Setting<double> d6;
    static settings::Setting<double> d7;

    static settings::Setting<bool> b1;
    static settings::Setting<bool> b2;
    static settings::Setting<bool> b3;
    static settings::Setting<bool> b4;
    static settings::Setting<bool> b5;
    static settings::Setting<bool> b6;
    static settings::Setting<bool> b7;

    static settings::Setting<void> root;
    static settings::Setting<int> rootInt1;
    static settings::Setting<int> rootInt2;
};

}  // namespace pajadog
