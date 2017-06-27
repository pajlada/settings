#pragma once

#include <rapidjson/document.h>

namespace pajlada {
namespace Settings {
namespace detail {

/// rapidjson deep merge code
void mergeObjects(rapidjson::Value &destination, rapidjson::Value &source,
                  rapidjson::Document::AllocatorType &allocator);
void mergeArrays(rapidjson::Value &destination, rapidjson::Value &source,
                 rapidjson::Document::AllocatorType &allocator);

void
mergeObjects(rapidjson::Value &destination, rapidjson::Value &source,
             rapidjson::Document::AllocatorType &allocator)
{
    assert(destination.IsObject());
    assert(source.IsObject());

    for (auto sourceIt = source.MemberBegin(); sourceIt != source.MemberEnd();
         ++sourceIt) {
        // Does the source member exist in destination?
        auto destinationIt = destination.FindMember(sourceIt->name);
        if (destinationIt == destination.MemberEnd()) {
            // Source member was not found in destination. Add member
            destination.AddMember(sourceIt->name, sourceIt->value, allocator);
            continue;
        }

        // Are the source member and destination member with the same key equal
        // types?
        auto sourceType = sourceIt->value.GetType();
        auto destinationType = destinationIt->value.GetType();

        if (sourceType != destinationType) {
            // The types are not the same
            // XXX(pajlada): What do we do in this scenario? For now, we
            // override destination member with source member
            destinationIt->value = sourceIt->value;
            continue;
        }

        // Source type and destination type are equal
        if (sourceType == rapidjson::kObjectType) {
            // Source and destination members are objects. Run mergeObjects
            // I hope we don't recurse our way to hell
            mergeObjects(destinationIt->value, sourceIt->value, allocator);
            continue;
        }

        if (sourceType == rapidjson::kArrayType) {
            // We have multiple options here
            // Do we merge the arrays, index by index? Or do we simply push back
            // any source member indices to the destination member
            // For now, we will merge index by index
            mergeArrays(destinationIt->value, sourceIt->value, allocator);
            continue;
        }

        destinationIt->value = sourceIt->value;
    }
}

void
mergeArrays(rapidjson::Value &destination, rapidjson::Value &source,
            rapidjson::Document::AllocatorType &allocator)
{
    assert(destination.IsArray());
    assert(source.IsArray());

    unsigned index = 0;
    for (auto sourceArrayIt = source.Begin(); sourceArrayIt != source.End();
         ++sourceArrayIt, ++index) {
        if (index < destination.Size()) {
            // Merge
            auto &destinationArrayValue = destination[index];

            auto sourceArrayValueType = sourceArrayIt->GetType();
            auto destinationArrayValueType = destinationArrayValue.GetType();

            if (sourceArrayValueType != destinationArrayValueType) {
                // The types are not the same
                // See comment above for what we should do in this case
                destinationArrayValue = *sourceArrayIt;
                continue;
            }

            if (sourceArrayValueType == rapidjson::kObjectType) {
                mergeObjects(destinationArrayValue, *sourceArrayIt, allocator);
                continue;
            }

            if (sourceArrayValueType == rapidjson::kArrayType) {
                mergeArrays(destinationArrayValue, *sourceArrayIt, allocator);
                continue;
            }

            // Same type, just replace the value
            destinationArrayValue = *sourceArrayIt;
        } else {
            // Append
            destination.PushBack(*sourceArrayIt, allocator);
        }
    }
}

}  // namespace detail
}  // namespace Settings
}  // namespace pajlada
