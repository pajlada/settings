#include "test/foo.hpp"

namespace pajlada {

using namespace Settings;

Setting<int> Foo::i1("rootSetting1", 1);
Setting<int> Foo::i2("rootSetting2", 2);
Setting<int> Foo::i3("rootSetting3", 3);
Setting<std::string> Foo::s1(
    "rootSetting4",
    "Default string");  // XXX(pajlada): Something is wrong with this

Setting<float> Foo::f1("float1", 1.0101f);
Setting<float> Foo::f2("float2", 1.0101010101f);
Setting<float> Foo::f3("float3", 1.123456789f);
Setting<float> Foo::f4("float4", 1.f);
Setting<float> Foo::f5("float5", 0.f);
Setting<float> Foo::f6("float6", -.1f);

Setting<double> Foo::d1("double1", 1.0101);
Setting<double> Foo::d2("double2", 1.0101010101);
Setting<double> Foo::d3("double3", 1.123456789);
Setting<double> Foo::d4("double4", 1.);
Setting<double> Foo::d5("double5", 0.);
Setting<double> Foo::d6("double6", -.1);
Setting<double> Foo::d7("double7", 123.456);

Setting<bool> Foo::b1("bool1", true);
Setting<bool> Foo::b2("bool2", false);
Setting<bool> Foo::b3("bool3", true);
Setting<bool> Foo::b4("bool4", false);
Setting<bool> Foo::b5("bool5", true);
Setting<bool> Foo::b6("bool6", false);
Setting<bool> Foo::b7("bool7", true);

Setting<int> Foo::rootInt1 = Setting<int>("/root/i1", 1);
// the error happens here for some reason
// these lines (i1 and i2) are the reason that the new array thing isn't saved
// properly
Setting<int> Foo::rootInt2 = Setting<int>("/root/i2", 1);

}  // namespace pajlada
