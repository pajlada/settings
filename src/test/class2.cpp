#include "test/class2.hpp"

namespace pajadog {

settings::Setting<int> Class2::i1("rootSetting1", 1);
settings::Setting<int> Class2::i2("rootSetting2", 2);
settings::Setting<int> Class2::i3("rootSetting3", 3);
settings::Setting<std::string> Class2::s1("rootSetting4", "Default string");
settings::Setting<float> Class2::f1("rootSetting5", 1.0101f);
settings::Setting<double> Class2::d1("rootSetting6", 2.0202);

}  // namespace pajadog
