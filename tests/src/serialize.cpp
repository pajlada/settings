#include <gtest/gtest.h>

#include <pajlada/settings.hpp>

#include "common.hpp"

using namespace pajlada::Settings;
using SaveResult = SettingManager::SaveResult;
using SaveMethod = SettingManager::SaveMethod;
using LoadError = SettingManager::LoadError;

namespace fs = std::filesystem;

namespace {

const fs::path PRE = "files/";

}  // namespace

TEST(Serialize, VectorBeforeLoading)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<std::vector<std::string>> a("/a", sm);
    auto vec = a.getValue();

    EXPECT_EQ(vec.size(), 0);
}

TEST(Serialize, VectorAfterSetting)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    std::vector<std::string> data{"a", "b", "c"};
    Setting<std::vector<std::string>> a("/a", sm);
    a = data;

    EXPECT_EQ(a.getValue().size(), data.size());
    EXPECT_EQ(a.getValue(), data);
}

TEST(Serialize, VectorAfterLoading)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<std::vector<std::string>> a("/a", sm);
    ASSERT_EQ(LoadError::NoError,
              sm->loadFrom("files/in.serialize.vector.str.json"));

    const auto &vec = a.getValue();

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], "x");
    EXPECT_EQ(vec[1], "D");
}

TEST(Serialize, VectorMisc)
{
    auto fOut = PRE / "out.serialize.vector.misc.json";
    auto fCmp = PRE / "expected.serialize.vector.misc.json";

    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<std::vector<std::string>> v("/Serialize.VectorMisc", sm);
    std::vector<std::string> newData{"l", "o", "l", "4HEad"};

    v = newData;

    ASSERT_EQ(SaveResult::Success, sm->saveAs(fOut));

    EXPECT_EQ(ReadFileE(fOut), ReadFileE(fCmp));
}

TEST(Serialize, StdAnyVectorString)
{
    auto fIn = PRE / "in.serialize.any.vector.str.json";
    auto fOut = PRE / "out.serialize.any.vector.str.json";
    auto fCmp = PRE / "expected.serialize.any.vector.str.json";

    RemoveFile(fOut);

    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    using std::any_cast;

    std::vector<std::string> data{"bing", "bang", "bong"};

    Setting<std::any> v("/Serialize.StdAnyVectorString", sm);

    auto rawAny = v.getValue();
    EXPECT_FALSE(rawAny.has_value());

    v = data;
    rawAny = v.getValue();
    EXPECT_TRUE(rawAny.has_value());
    {
        auto vec = any_cast<std::vector<std::any>>(rawAny);
        EXPECT_EQ(vec.size(), data.size());
        EXPECT_EQ(any_cast<std::string>(vec[0]), data[0]);
        EXPECT_EQ(any_cast<std::string>(vec[1]), data[1]);
        EXPECT_EQ(any_cast<std::string>(vec[2]), data[2]);
    }

    ASSERT_EQ(LoadError::NoError, sm->loadFrom(fIn));

    rawAny = v.getValue();
    EXPECT_TRUE(rawAny.has_value());
    {
        auto vec = any_cast<std::vector<std::any>>(rawAny);
        EXPECT_EQ(vec.size(), 2);
        EXPECT_EQ(any_cast<std::string>(vec[0]), "x");
        EXPECT_EQ(any_cast<std::string>(vec[1]), "D");
    }

    std::vector<std::string> newData{"bing", "bang", "bong"};

    v = newData;

    EXPECT_EQ(SaveResult::Success, sm->saveAs(fOut));

    EXPECT_EQ(ReadFileE(fOut), ReadFileE(fCmp));
}

TEST(Serialize, StdAnyVectorAny)
{
    auto fIn = PRE / "in.serialize.any.vector.any.json";
    auto fOut = PRE / "out.serialize.any.vector.any.json";
    auto fCmp = PRE / "expected.serialize.any.vector.any.json";

    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    using std::any_cast;

    std::vector<std::any> data{"test", 5, 13.37};

    Setting<std::any> v("/Serialize.StdAnyVectorAny", sm);

    auto rawAny = v.getValue();
    ASSERT_FALSE(rawAny.has_value());

    v = data;

    rawAny = v.getValue();
    {
        auto vec = any_cast<std::vector<std::any>>(rawAny);

        ASSERT_EQ(vec.size(), data.size());

        ASSERT_EQ(vec[0].type().name(), typeid(std::string).name());
        ASSERT_EQ(any_cast<std::string>(vec[0]), "test");
        ASSERT_EQ(any_cast<int>(vec[1]), 5);
        ASSERT_DOUBLE_EQ(any_cast<double>(vec[2]), 13.37);
    }

    ASSERT_EQ(LoadError::NoError, sm->loadFrom(fIn));

    rawAny = v.getValue();
    {
        auto vec = any_cast<std::vector<std::any>>(rawAny);

        ASSERT_EQ(vec.size(), 2);
        ASSERT_EQ(any_cast<std::string>(vec[0]), "foo");
        ASSERT_DOUBLE_EQ(any_cast<double>(vec[1]), 4.20);
    }

    std::vector<std::any> newData{"forsen", 5};

    v = newData;

    ASSERT_EQ(SaveResult::Success, sm->saveAs(fOut));

    ASSERT_EQ(ReadFileE(fOut), ReadFileE(fCmp));
}

TEST(Serialize, Int1)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/a", sm);
    EXPECT_EQ(a, 0);
    EXPECT_EQ(a.getValue(), 0);
    int val = a;
    EXPECT_EQ(val, 0);
}

TEST(Serialize, Int2)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    int data = 8;
    Setting<int> a("/a", sm);
    a = data;

    EXPECT_EQ(a, data);
    EXPECT_EQ(a.getValue(), data);
    int val = a;
    EXPECT_EQ(val, data);
}

TEST(Serialize, Int3)
{
    auto fIn = PRE / "in.serialize.int.json";

    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/a", sm);
    ASSERT_EQ(LoadError::NoError, sm->loadFrom(fIn));
    ASSERT_EQ(a, 10);
    ASSERT_EQ(a.getValue(), 10);
    int val = a;
    ASSERT_EQ(val, 10);
}

TEST(Serialize, Int4)
{
    auto fIn = PRE / "in.serialize.int4.json";
    auto fOut = PRE / "out.serialize.int4.json";
    auto fCmp = PRE / "expected.serialize.int4.json";

    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    Setting<int> a("/Serialize.Int4", sm);

    ASSERT_EQ(a, 0);

    ASSERT_EQ(LoadError::NoError, sm->loadFrom(fIn));

    ASSERT_EQ(a, 10);

    a = 25;

    ASSERT_EQ(a, 25);

    EXPECT_EQ(SaveResult::Success, sm->saveAs(fOut));

    EXPECT_EQ(ReadFileE(fOut), ReadFileE(fCmp));
}

TEST(Serialize, Bool)
{
    auto sm = std::make_shared<SettingManager>();
    sm->saveMethod = SaveMethod::SaveManually;

    bool data = true;
    bool val;

    Setting<bool> a("/a", sm);

    EXPECT_EQ(a, false);
    EXPECT_EQ(a.getValue(), false);
    val = a;
    EXPECT_EQ(val, false);

    a = data;

    EXPECT_EQ(a, data);
    EXPECT_EQ(a.getValue(), data);
    val = a;
    EXPECT_EQ(val, data);

    a = false;

    EXPECT_EQ(a, false);
    EXPECT_EQ(a.getValue(), false);
    val = a;
    EXPECT_EQ(val, false);

    ASSERT_EQ(LoadError::NoError, sm->loadFrom("files/in.serialize.bool.json"));
    EXPECT_EQ(a, true);
    EXPECT_EQ(a.getValue(), true);
    val = a;
    EXPECT_EQ(val, true);

    ASSERT_EQ(LoadError::NoError,
              sm->loadFrom("files/in.serialize.bool2.json"));
    EXPECT_EQ(a, false);
    EXPECT_EQ(a.getValue(), false);
    val = a;
    EXPECT_EQ(val, false);

    ASSERT_EQ(LoadError::NoError,
              sm->loadFrom("files/in.serialize.bool3.json"));
    EXPECT_EQ(a, true);
    EXPECT_EQ(a.getValue(), true);
    val = a;
    EXPECT_EQ(val, true);

    ASSERT_EQ(LoadError::NoError,
              sm->loadFrom("files/in.serialize.bool4.json"));
    EXPECT_EQ(a, false);
    EXPECT_EQ(a.getValue(), false);
    val = a;
    EXPECT_EQ(val, false);

    a = true;
    EXPECT_EQ(a, true);
    EXPECT_EQ(a.getValue(), true);
    val = a;
    EXPECT_EQ(val, true);
    EXPECT_EQ(SaveResult::Success, sm->saveAs("files/out.serialize.bool.json"));
    EXPECT_TRUE(
        FilesMatch("in.serialize.bool.json", "out.serialize.bool.json"));

    a = false;
    EXPECT_EQ(a, false);
    EXPECT_EQ(a.getValue(), false);
    val = a;
    EXPECT_EQ(val, false);
    EXPECT_EQ(SaveResult::Success, sm->saveAs("files/out.serialize.bool.json"));
    EXPECT_TRUE(
        FilesMatch("in.serialize.bool.false.json", "out.serialize.bool.json"));
}
