#include <iostream>

#include "common.hpp"

using namespace pajlada::Settings;
using SaveResult = pajlada::Settings::SettingManager::SaveResult;

TEST(Serialize, VectorBeforeLoading)
{
    std::vector<std::string> data{"a", "b", "c"};
    Setting<std::vector<std::string>> a("/a");
    auto vec = a.getValue();

    EXPECT_TRUE(vec.size() == 0);
}

TEST(Serialize, VectorAfterSetting)
{
    std::vector<std::string> data{"a", "b", "c"};
    Setting<std::vector<std::string>> a("/a");
    a = data;

    auto vec = a.getValue();

    EXPECT_TRUE(vec.size() == data.size());
}

TEST(Serialize, VectorAfterLoading)
{
    std::vector<std::string> data{"a", "b", "c"};
    Setting<std::vector<std::string>> a("/a");
    EXPECT_TRUE(LoadFile("in.serialize.vector.str.json"));
    auto vec = a.getValue();

    EXPECT_TRUE(vec.size() == 2);
    EXPECT_TRUE(vec[0] == "x");
    EXPECT_TRUE(vec[1] == "D");
}

TEST(Serialize, VectorMisc)
{
    std::vector<std::string> data{"a", "b", "c"};
    Setting<std::vector<std::string>> a("/a");
    std::vector<std::string> newData{"l", "o", "l", "4HEad"};

    a = newData;

    EXPECT_EQ(SaveResult::Success, SaveFile("out.serialize.vector.str.json"));

    EXPECT_TRUE(FilesMatch("in.serialize.vector.str.state1.json",
                           "out.serialize.vector.str.json"));
}

TEST(Serialize, StdAnyVectorString)
{
    using std::any_cast;

    SettingManager::clear();

    std::vector<std::string> data{"a", "b", "c"};

    Setting<std::any> a("/a");

    auto rawAny = a.getValue();
    EXPECT_FALSE(rawAny.has_value());

    a = data;

    rawAny = a.getValue();

    EXPECT_TRUE(rawAny.has_value());

    auto vec = any_cast<std::vector<std::any>>(rawAny);

    EXPECT_TRUE(vec.size() == data.size());

    EXPECT_TRUE(LoadFile("in.serialize.any.vector.str.json"));

    rawAny = a.getValue();
    vec = any_cast<std::vector<std::any>>(rawAny);

    EXPECT_TRUE(vec.size() == 2);
    EXPECT_TRUE(any_cast<std::string>(vec[0]) == "x");
    EXPECT_TRUE(any_cast<std::string>(vec[1]) == "D");

    std::vector<std::string> newData{"l", "o", "l", "4HEad"};

    a = newData;

    EXPECT_EQ(SaveResult::Success,
              SaveFile("out.serialize.any.vector.str.json"));

    EXPECT_TRUE(FilesMatch("in.serialize.vector.str.state1.json",
                           "out.serialize.any.vector.str.json"));
}

TEST(Serialize, StdAnyVectorAny)
{
    using std::any_cast;

    SettingManager::clear();

    std::vector<std::any> data{"test", 5, 13.37};

    Setting<std::any> a("/a");

    auto rawAny = a.getValue();
    EXPECT_FALSE(rawAny.has_value());

    a = data;

    rawAny = a.getValue();
    auto vec = any_cast<std::vector<std::any>>(rawAny);

    EXPECT_TRUE(vec.size() == data.size());

    EXPECT_TRUE(vec[0].type().name() == typeid(std::string).name());
    EXPECT_TRUE(any_cast<std::string>(vec[0]) == "test");
    EXPECT_TRUE(any_cast<int>(vec[1]) == 5);
    EXPECT_DOUBLE_EQ(any_cast<double>(vec[2]), 13.37);

    EXPECT_TRUE(LoadFile("in.serialize.any.vector.str.json"));

    rawAny = a.getValue();
    vec = any_cast<std::vector<std::any>>(rawAny);

    EXPECT_TRUE(vec.size() == 2);
    EXPECT_TRUE(any_cast<std::string>(vec[0]) == "x");
    EXPECT_TRUE(any_cast<std::string>(vec[1]) == "D");

    std::vector<std::string> newData{"l", "o", "l", "4HEad"};

    a = newData;

    EXPECT_EQ(SaveResult::Success,
              SaveFile("out.serialize.any.vector.str.json"));

    EXPECT_TRUE(FilesMatch("in.serialize.vector.str.state1.json",
                           "out.serialize.any.vector.str.json"));
}

TEST(Serialize, Int1)
{
    SettingManager::clear();
    Setting<int> a("/a");
    EXPECT_TRUE(a == 0);
    EXPECT_TRUE(a.getValue() == 0);
    int val = a;
    EXPECT_TRUE(val == 0);
}

TEST(Serialize, Int2)
{
    SettingManager::clear();
    int data = 8;
    Setting<int> a("/a");
    a = data;

    EXPECT_TRUE(a == data);
    EXPECT_TRUE(a.getValue() == data);
    int val = a;
    EXPECT_TRUE(val == data);
}

TEST(Serialize, Int3)
{
    SettingManager::clear();
    Setting<int> a("/a");
    EXPECT_TRUE(LoadFile("in.serialize.int.json"));
    EXPECT_TRUE(a == 10);
    EXPECT_TRUE(a.getValue() == 10);
    int val = a;
    EXPECT_TRUE(val == 10);
}

TEST(Serialize, Int4)
{
    SettingManager::clear();
    Setting<int> a("/a");
    EXPECT_TRUE(LoadFile("in.serialize.int.json"));
    EXPECT_EQ(SaveResult::Success, SaveFile("out.serialize.int.json"));

    EXPECT_TRUE(FilesMatch("in.serialize.int.json", "out.serialize.int.json"));
}

TEST(Serialize, Bool)
{
    SettingManager::clear();

    bool data = true;
    bool val;

    Setting<bool> a("/a");

    EXPECT_TRUE(a == false);
    EXPECT_TRUE(a.getValue() == false);
    val = a;
    EXPECT_TRUE(val == false);

    a = data;

    EXPECT_TRUE(a == data);
    EXPECT_TRUE(a.getValue() == data);
    val = a;
    EXPECT_TRUE(val == data);

    a = false;

    EXPECT_TRUE(a == false);
    EXPECT_TRUE(a.getValue() == false);
    val = a;
    EXPECT_TRUE(val == false);

    EXPECT_TRUE(LoadFile("in.serialize.bool.json"));
    EXPECT_TRUE(a == true);
    EXPECT_TRUE(a.getValue() == true);
    val = a;
    EXPECT_TRUE(val == true);

    EXPECT_TRUE(LoadFile("in.serialize.bool2.json"));
    EXPECT_TRUE(a == false);
    EXPECT_TRUE(a.getValue() == false);
    val = a;
    EXPECT_TRUE(val == false);

    EXPECT_TRUE(LoadFile("in.serialize.bool3.json"));
    EXPECT_TRUE(a == true);
    EXPECT_TRUE(a.getValue() == true);
    val = a;
    EXPECT_TRUE(val == true);

    EXPECT_TRUE(LoadFile("in.serialize.bool4.json"));
    EXPECT_TRUE(a == false);
    EXPECT_TRUE(a.getValue() == false);
    val = a;
    EXPECT_TRUE(val == false);

    a = true;
    EXPECT_TRUE(a == true);
    EXPECT_TRUE(a.getValue() == true);
    val = a;
    EXPECT_TRUE(val == true);
    EXPECT_EQ(SaveResult::Success, SaveFile("out.serialize.bool.json"));
    EXPECT_TRUE(
        FilesMatch("in.serialize.bool.json", "out.serialize.bool.json"));

    a = false;
    EXPECT_TRUE(a == false);
    EXPECT_TRUE(a.getValue() == false);
    val = a;
    EXPECT_TRUE(val == false);
    EXPECT_EQ(SaveResult::Success, SaveFile("out.serialize.bool.json"));
    EXPECT_TRUE(
        FilesMatch("in.serialize.bool.false.json", "out.serialize.bool.json"));
}
