#include <gtest/gtest.h>

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

TEST(Static, Static)
{
    ssm->setBackupEnabled(true);
    EXPECT_TRUE(iNoDefault.getValue() == 0);
    EXPECT_TRUE(iDefault.getValue() == 5);

    EXPECT_TRUE(f1.getValue() == 1.0101f);
    EXPECT_TRUE(f2.getValue() == 1.0101010101f);
    EXPECT_TRUE(f3.getValue() == 1.123456789f);
    EXPECT_TRUE(f4.getValue() == 1.f);
    EXPECT_TRUE(f5.getValue() == 0.f);
    EXPECT_TRUE(f6.getValue() == -.1f);
    EXPECT_TRUE(f7.getValue() == 1.f);

    EXPECT_TRUE(d1.getValue() == 1.0101);
    EXPECT_TRUE(d2.getValue() == 1.0101010101);
    EXPECT_TRUE(d3.getValue() == 1.123456789);
    EXPECT_TRUE(d4.getValue() == 1.);
    EXPECT_TRUE(d5.getValue() == 0.);
    EXPECT_TRUE(d6.getValue() == -.1);
    EXPECT_TRUE(d7.getValue() == 123.456);

    EXPECT_TRUE(b1.getValue() == true);
    EXPECT_TRUE(b2.getValue() == false);
    EXPECT_TRUE(b3.getValue() == true);
    EXPECT_TRUE(b4.getValue() == false);
    EXPECT_TRUE(b5.getValue() == true);
    EXPECT_TRUE(b6.getValue() == false);
    EXPECT_TRUE(b7.getValue() == true);

    EXPECT_TRUE(LoadFile("test.json", ssm.get()));

    // Floats post-load
    EXPECT_TRUE(f1.getValue() == 1.f);
    EXPECT_TRUE(f2.getValue() == -1.f);
    EXPECT_TRUE(f3.getValue() == 500.987654321f);
    EXPECT_TRUE(f4.getValue() == 5.1f);
    EXPECT_TRUE(f5.getValue() == 5.321f);
    EXPECT_TRUE(f6.getValue() == 500000.1f);

    // Doubles post-load
    EXPECT_TRUE(d1.getValue() == 1.);
    EXPECT_TRUE(d2.getValue() == -1.);
    EXPECT_TRUE(d3.getValue() == 500.987654321);
    EXPECT_TRUE(d4.getValue() == 5.1);
    EXPECT_TRUE(d5.getValue() == 5.321);
    EXPECT_TRUE(d6.getValue() == 500000.1);
    EXPECT_TRUE(d7.getValue() == 123.456);

    // Booleans post-load
    EXPECT_TRUE(b1.getValue() == false);  // false
    EXPECT_TRUE(b2.getValue() == true);   // true
    EXPECT_TRUE(b3.getValue() == false);  // 0
    EXPECT_TRUE(b4.getValue() == true);   // 1
    EXPECT_TRUE(b5.getValue() == false);  // 50291 (anything but 1)
    EXPECT_TRUE(b6.getValue() == false);  // nothing loaded
    EXPECT_TRUE(b7.getValue() == true);   // nothing loaded

    EXPECT_TRUE(iNoDefault.getValue() == 2);
    EXPECT_TRUE(iDefault.getValue() == 3);

    // Class2::i1.signalValueChanged.connect([](const int &newValue) {
    //     std::cout << "new value for i1: " << newValue << std::endl;
    // });

    iNoDefault.setValue(3);

    EXPECT_TRUE(iNoDefault.getValue() == 3);

    iNoDefault = 4;

    EXPECT_TRUE(iNoDefault.getValue() == 4);

    f7 = 0.f;

    EXPECT_TRUE(SaveFile("out.test2.json", ssm.get()));
}
