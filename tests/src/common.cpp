#include "common.hpp"

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <fstream>
#include <iostream>

using namespace pajlada::Settings;

namespace {

std::string initialPath = "files/";

}  // namespace

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

std::string
ReadFileE(const std::filesystem::path &path)
{
    std::ifstream fh(path, std::ios::in | std::ios::binary);
    if (!fh) {
        throw std::runtime_error("file at " + path.string() + " is empty");
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

    // std::cout << path1 << ": '" << content1 << "'\n";
    // std::cout << path2 << ": '" << content2 << "'\n";

    return content1 == content2;
}

bool
RemoveFile(const std::filesystem::path &path)
{
    std::error_code ec;
    auto res = std::filesystem::remove(path, ec);

    if (ec) {
        return false;
    }

    return res;
}

void
RJPrettyPrint(const std::shared_ptr<pajlada::Settings::SettingManager> &sm,
              const std::string &prefix)
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    sm->document.Accept(writer);

    std::cout << prefix << buffer.GetString() << '\n';
}

std::string
RJStringify(const rapidjson::Value &v)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);

    return {buffer.GetString()};
}
