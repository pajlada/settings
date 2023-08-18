#pragma once

#include <rapidjson/pointer.h>

#include <any>
#include <iostream>
#include <map>
#include <string>

namespace pajlada {
namespace Settings {
namespace detail {

static void
loadObject(const rapidjson::Value &node, std::string path,
           std::map<std::string, std::any> &data)
{
    // auto path = prefix + node.name.GetString();
    if (node.IsBool()) {
        data[path] = node.GetBool();
    } else if (node.IsInt()) {
        data[path] = node.GetInt();
    } else if (node.IsUint()) {
        data[path] = node.GetUint();
    } else if (node.IsInt64()) {
        data[path] = node.GetInt64();
    } else if (node.IsUint64()) {
        data[path] = node.GetUint64();
    } else if (node.IsDouble()) {
        data[path] = node.GetDouble();
    } else if (node.IsString()) {
        data[path] = std::string(node.GetString());
    } else if (node.IsArray()) {
        for (rapidjson::SizeType i = 0; i < node.Size(); ++i) {
            loadObject(node[i], path + "/" + std::to_string(i), data);
        }
    } else if (node.IsObject()) {
        for (auto objectNode = node.MemberBegin();
             objectNode != node.MemberEnd(); ++objectNode) {
            loadObject(objectNode->value,
                       path + "/" + objectNode->name.GetString(), data);
        }
    }
}

}  // namespace detail
}  // namespace Settings
}  // namespace pajlada
