#pragma once

#include <gtest/gtest.h>

#include <pajlada/settings.hpp>
#include <string>

std::string ReadFile(const std::string &path);

bool FilesMatch(const std::string &fileName1, const std::string &fileName2);

bool LoadFile(const std::string &fileName,
              pajlada::Settings::SettingManager *sm = nullptr);
bool SaveFile(const std::string &fileName,
              pajlada::Settings::SettingManager *sm = nullptr);

bool RemoveFile(const std::string &path);

#ifdef PAJLADA_SETTINGS_ENABLE_EXCEPTIONS
#define DD_THROWS(x) REQUIRE_THROWS(x)
#define REQUIRE_IF_NOEXCEPT(x, y)
#define REQUIRE_IF_NOEXCEPT2(x, y)
#else
#define DD_THROWS(x) x
#define REQUIRE_IF_NOEXCEPT(x, y) EXPECT_TRUE(x->getValue() == y)
#define REQUIRE_IF_NOEXCEPT2(x, y) EXPECT_TRUE(x == y)
#endif
