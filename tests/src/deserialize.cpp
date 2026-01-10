#include <gtest/gtest.h>

#include <pajlada/settings.hpp>

#include "common.hpp"

using namespace pajlada::Settings;
using SaveResult = SettingManager::SaveResult;
using SaveMethod = SettingManager::SaveMethod;
using LoadError = SettingManager::LoadError;
using namespace pajlada;

const double EPSILON = 1e-6;

class FDeserialize : public ::testing::Test
{
protected:
    void
    SetUp() override
    {
        this->sm = std::make_shared<SettingManager>();
        this->sm->saveMethod = SaveMethod::SaveManually;
        ASSERT_EQ(LoadError::NoError,
                  this->sm->loadFrom("files/deserialize-samples.json"));
    }

    void
    TearDown() override
    {
        this->sm.reset();
    }

    std::shared_ptr<SettingManager> sm;
};

TEST_F(FDeserialize, A)
{
}

TEST_F(FDeserialize, Int)
{
    Setting<int> a1("/int", this->sm);
    Setting<int> a2("/float", this->sm);
    std::unique_ptr<Setting<int>> a3(
        new Setting<int>("/std::string", this->sm));
    std::unique_ptr<Setting<int>> a4(new Setting<int>("/bool", this->sm));

    EXPECT_TRUE(a1.getValue() == 5);
    EXPECT_TRUE(a2.getValue() == 5);
    DD_THROWS(a3->getValue());
    DD_THROWS(a4->getValue());
}

TEST_F(FDeserialize, Float)
{
    Setting<float> a1("/int", this->sm);
    Setting<float> a2("/float", this->sm);
    std::unique_ptr<Setting<float>> a3(
        new Setting<float>("/std::string", this->sm));
    std::unique_ptr<Setting<float>> a4(new Setting<float>("/bool", this->sm));

    EXPECT_NEAR(a1.getValue(), 5.f, EPSILON);
    EXPECT_NEAR(a2.getValue(), 5.3f, EPSILON);
    DD_THROWS(a3->getValue());
    DD_THROWS(a4->getValue());
}

TEST_F(FDeserialize, Double)
{
    Setting<double> a1("/int", this->sm);
    Setting<double> a2("/float", this->sm);
    std::unique_ptr<Setting<double>> a3(
        new Setting<double>("/std::string", this->sm));
    std::unique_ptr<Setting<double>> a4(new Setting<double>("/bool", this->sm));

    EXPECT_NEAR(a1.getValue(), 5., EPSILON);
    EXPECT_NEAR(a2.getValue(), 5.3, EPSILON);
    DD_THROWS(a3->getValue());
    DD_THROWS(a4->getValue());
}

TEST_F(FDeserialize, Bool)
{
    Setting<bool> a1("/int", this->sm);
    std::unique_ptr<Setting<bool>> a2(new Setting<bool>("/float", this->sm));
    std::unique_ptr<Setting<bool>> a3(
        new Setting<bool>("/std::string", this->sm));
    std::unique_ptr<Setting<bool>> a4(new Setting<bool>("/bool", this->sm));

    Setting<bool> a5("/int1", this->sm);

    EXPECT_TRUE(a1.getValue() == false);
    DD_THROWS(a2->getValue());
    DD_THROWS(a3->getValue());
    EXPECT_TRUE(a4->getValue() == true);
    EXPECT_TRUE(a5.getValue() == true);
}

TEST_F(FDeserialize, String)
{
    std::unique_ptr<Setting<std::string>> a1(
        new Setting<std::string>("/int", this->sm));
    std::unique_ptr<Setting<std::string>> a2(
        new Setting<std::string>("/float", this->sm));
    std::unique_ptr<Setting<std::string>> a3(
        new Setting<std::string>("/string", this->sm));
    std::unique_ptr<Setting<std::string>> a4(
        new Setting<std::string>("/bool", this->sm));

    DD_THROWS(a1->getValue());
    DD_THROWS(a2->getValue());
    EXPECT_TRUE(a3->getValue() == "xd");
    DD_THROWS(a4->getValue());
}

class FDeserialize2 : public ::testing::Test
{
protected:
    void
    SetUp() override
    {
    }

    void
    TearDown() override
    {
    }

    rapidjson::Value jString{"asd"};
    rapidjson::Value jInt{5};
    rapidjson::Value jBool{true};
    rapidjson::Value jDouble{6.4};

    bool error = false;
};

TEST_F(FDeserialize2, Int)
{
    int v = 0;

    error = false;
    DD_THROWS(v = Deserialize<int>::get(jString, &error));
    EXPECT_TRUE(error == true);
    REQUIRE_IF_NOEXCEPT2(v, 0);

    error = false;
    v = Deserialize<int>::get(jInt, &error);
    EXPECT_TRUE(error == false);
    EXPECT_TRUE(v == 5);

    error = false;
    DD_THROWS(v = Deserialize<int>::get(jBool, &error));
    EXPECT_TRUE(error == true);
    REQUIRE_IF_NOEXCEPT2(v, 0);

    error = false;
    v = Deserialize<int>::get(jDouble, &error);
    EXPECT_TRUE(error == false);
    EXPECT_TRUE(v == 6);
}

TEST_F(FDeserialize2, String)
{
    std::string v = "";

    error = false;
    v = Deserialize<std::string>::get(this->jString, &error);
    EXPECT_TRUE(error == false);
    EXPECT_TRUE(v == "asd");

    error = false;
    DD_THROWS(v = Deserialize<std::string>::get(jInt, &error));
    EXPECT_TRUE(error == true);
    REQUIRE_IF_NOEXCEPT2(v, "");

    error = false;
    DD_THROWS(v = Deserialize<std::string>::get(jBool, &error));
    EXPECT_TRUE(error == true);
    REQUIRE_IF_NOEXCEPT2(v, "");

    error = false;
    DD_THROWS(v = Deserialize<std::string>::get(this->jDouble, &error));
    EXPECT_TRUE(error == true);
    REQUIRE_IF_NOEXCEPT2(v, "");
}
