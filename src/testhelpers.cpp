#include "testhelpers.hpp"

#include <pajlada/settings/settingmanager.hpp>

#include <fstream>
#include <iostream>

static std::string initialPath = "files/";

std::string
ReadFile(const std::string &path)
{
    std::ifstream fh(path, std::ios::in | std::ios::binary);
    if (!fh) {
        return "";
    }

    std::string contents;
    fh.seekg(0, std::ios::end);
    contents.resize(fh.tellg());
    fh.seekg(0, std::ios::beg);
    fh.read(&contents[0], contents.size());
    fh.close();

    return contents;
}

bool
FilesMatch(const std::string &fileName1, const std::string &fileName2)
{
    std::string path1 = initialPath + fileName1;
    std::string path2 = initialPath + fileName2;

    auto content1 = ReadFile(path1);
    auto content2 = ReadFile(path2);

    // std::cout << path1 << ": " << content1 << '\n';
    // std::cout << path2 << ": " << content2 << '\n';

    return content1 == content2;
}

bool
LoadFile(const std::string &fileName)
{
    using namespace pajlada::Settings;

    std::string path = initialPath + fileName;

    return SettingManager::loadFrom(path.c_str()) ==
           SettingManager::LoadError::NoError;
}

bool
SaveFile(const std::string &fileName)
{
    using namespace pajlada::Settings;

    std::string path = initialPath + fileName;

    return SettingManager::saveAs(path.c_str());
}
