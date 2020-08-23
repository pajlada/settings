#include "test/common.hpp"

#include <fstream>
#include <iostream>

#include "pajlada/settings/detail/fs.hpp"

using namespace pajlada::Settings;
using namespace std;

namespace {

string initialPath = "files/";

}  // namespace

string
ReadFile(const string &path)
{
    ifstream fh(path, ios::in | ios::binary);
    if (!fh) {
        return "";
    }

    string contents;
    fh.seekg(0, ios::end);
    contents.resize(fh.tellg());
    fh.seekg(0, ios::beg);
    fh.read(&contents[0], contents.size());
    fh.close();

    return contents;
}

bool
FilesMatch(const string &fileName1, const string &fileName2)
{
    string path1 = initialPath + fileName1;
    string path2 = initialPath + fileName2;

    auto content1 = ReadFile(path1);
    auto content2 = ReadFile(path2);

    // cout << path1 << ": " << content1 << '\n';
    // cout << path2 << ": " << content2 << '\n';

    return content1 == content2;
}

bool
LoadFile(const string &fileName, SettingManager *sm)
{
    if (sm == nullptr) {
        sm = SettingManager::getInstance().get();
    }

    string path = initialPath + fileName;

    return sm->loadFrom(path.c_str()) == SettingManager::LoadError::NoError;
}

bool
SaveFile(const string &fileName, SettingManager *sm)
{
    if (sm == nullptr) {
        sm = SettingManager::getInstance().get();
    }

    string path = initialPath + fileName;

    return sm->saveAs(path.c_str());
}

bool
RemoveFile(const std::string &path)
{
    fs_error_code ec;
    auto res = fs::remove(path, ec);

    if (ec) {
        return false;
    }

    return res;
}
