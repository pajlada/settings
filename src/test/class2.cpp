#include "test/class2.hpp"

namespace pajadog {

using namespace settings;

Setting<int> Class2::i1("rootSetting1", 1);
Setting<int> Class2::i2("rootSetting2", 2);
Setting<int> Class2::i3("rootSetting3", 3);
Setting<std::string> Class2::s1("rootSetting4", "Default string");
Setting<float> Class2::f1("rootSetting5", 1.0101f);
Setting<double> Class2::d1("rootSetting6", 2.0202);

Setting<void> Class2::root("root");
Setting<int> Class2::rootInt1 = Setting<int>("i1", 1, &Class2::root);
Setting<int> Class2::rootInt2 = Setting<int>("i2", 1, &Class2::root);

}  // namespace pajadog
