#include "test/class1.hpp"

#include <iostream>

namespace pajadog {

Setting Class1::test(new SettingData<int>("Class 1 setting", 5));

Class1::Class1()
{
    std::cout << "ctor Class1" << std::endl;
}

Class1::~Class1()
{
    std::cout << "dtor Class1" << std::endl;
}

}  // namespace pajadog
