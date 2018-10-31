#include "test/common.hpp"

using namespace pajlada::Settings;
using namespace std;

namespace {

// The static settings have been delegated to their own setting manager to not mess up other tests
auto ssm = std::make_shared<SettingManager>();

Setting<int> iNoDefault("/iNoDefault", ssm);
Setting<int> iDefault("/iDefault", 5, ssm);

Setting<float> f1("/float1", 1.0101f, ssm);
Setting<float> f2("/float2", 1.0101010101f, ssm);
Setting<float> f3("/float3", 1.123456789f, ssm);
Setting<float> f4("/float4", 1.f, ssm);
Setting<float> f5("/float5", 0.f, ssm);
Setting<float> f6("/float6", -.1f, ssm);
Setting<float> f7("/float7", 1.0f, ssm);

Setting<double> d1("/double1", 1.0101, ssm);
Setting<double> d2("/double2", 1.0101010101, ssm);
Setting<double> d3("/double3", 1.123456789, ssm);
Setting<double> d4("/double4", 1., ssm);
Setting<double> d5("/double5", 0., ssm);
Setting<double> d6("/double6", -.1, ssm);
Setting<double> d7("/double7", 123.456, ssm);

Setting<bool> b1("/bool1", true, ssm);
Setting<bool> b2("/bool2", false, ssm);
Setting<bool> b3("/bool3", true, ssm);
Setting<bool> b4("/bool4", false, ssm);
Setting<bool> b5("/bool5", true, ssm);
Setting<bool> b6("/bool6", false, ssm);
Setting<bool> b7("/bool7", true, ssm);

}  // namespace

TEST_CASE("static", "[settings][static]")
{
    REQUIRE(iNoDefault.getValue() == 0);
    REQUIRE(iDefault.getValue() == 5);

    REQUIRE(f1.getValue() == 1.0101f);
    REQUIRE(f2.getValue() == 1.0101010101f);
    REQUIRE(f3.getValue() == 1.123456789f);
    REQUIRE(f4.getValue() == 1.f);
    REQUIRE(f5.getValue() == 0.f);
    REQUIRE(f6.getValue() == -.1f);
    REQUIRE(f7.getValue() == 1.f);

    REQUIRE(d1.getValue() == 1.0101);
    REQUIRE(d2.getValue() == 1.0101010101);
    REQUIRE(d3.getValue() == 1.123456789);
    REQUIRE(d4.getValue() == 1.);
    REQUIRE(d5.getValue() == 0.);
    REQUIRE(d6.getValue() == -.1);
    REQUIRE(d7.getValue() == 123.456);

    REQUIRE(b1.getValue() == true);
    REQUIRE(b2.getValue() == false);
    REQUIRE(b3.getValue() == true);
    REQUIRE(b4.getValue() == false);
    REQUIRE(b5.getValue() == true);
    REQUIRE(b6.getValue() == false);
    REQUIRE(b7.getValue() == true);

    REQUIRE(LoadFile("test.json", ssm.get()));

    // Floats post-load
    REQUIRE(f1.getValue() == 1.f);
    REQUIRE(f2.getValue() == -1.f);
    REQUIRE(f3.getValue() == 500.987654321f);
    REQUIRE(f4.getValue() == 5.1f);
    REQUIRE(f5.getValue() == 5.321f);
    REQUIRE(f6.getValue() == 500000.1f);

    // Doubles post-load
    REQUIRE(d1.getValue() == 1.);
    REQUIRE(d2.getValue() == -1.);
    REQUIRE(d3.getValue() == 500.987654321);
    REQUIRE(d4.getValue() == 5.1);
    REQUIRE(d5.getValue() == 5.321);
    REQUIRE(d6.getValue() == 500000.1);
    REQUIRE(d7.getValue() == 123.456);

    // Booleans post-load
    REQUIRE(b1.getValue() == false);  // false
    REQUIRE(b2.getValue() == true);   // true
    REQUIRE(b3.getValue() == false);  // 0
    REQUIRE(b4.getValue() == true);   // 1
    REQUIRE(b5.getValue() == false);  // 50291 (anything but 1)
    REQUIRE(b6.getValue() == false);  // nothing loaded
    REQUIRE(b7.getValue() == true);   // nothing loaded

    REQUIRE(iNoDefault.getValue() == 2);
    REQUIRE(iDefault.getValue() == 3);

    // Class2::i1.signalValueChanged.connect([](const int &newValue) {
    //     std::cout << "new value for i1: " << newValue << std::endl;
    // });

    iNoDefault.setValue(3);

    REQUIRE(iNoDefault.getValue() == 3);

    iNoDefault = 4;

    REQUIRE(iNoDefault.getValue() == 4);

    f7 = 0.f;

    REQUIRE(SaveFile("out.test2.json", ssm.get()));
}
