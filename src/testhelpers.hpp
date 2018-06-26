#pragma once

#include <string>

std::string ReadFile(const std::string &path);

bool FilesMatch(const std::string &fileName1, const std::string &fileName2);

bool LoadFile(const std::string &fileName);
bool SaveFile(const std::string &fileName);

#ifdef PAJLADA_SETTINGS_ENABLE_EXCEPTIONS
#define DD_THROWS(x) REQUIRE_THROWS(x)
#define REQUIRE_IF_NOEXCEPT(x, y)
#else
#define DD_THROWS(x) x
#define REQUIRE_IF_NOEXCEPT(x, y) REQUIRE(x->getValue() == y)
#endif
