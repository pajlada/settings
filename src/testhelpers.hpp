#pragma once

#include <string>

std::string ReadFile(const std::string &path);

bool FilesMatch(const std::string &fileName1, const std::string &fileName2);

bool LoadFile(const std::string &fileName);
bool SaveFile(const std::string &fileName);
