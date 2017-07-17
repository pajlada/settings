#pragma once

#include <boost/any.hpp>

#include <map>
#include <vector>

namespace pajlada {
namespace Settings {

template <typename Type>
struct IsEqual {
    static bool
    get(const Type &lhs, const Type &rhs)
    {
        return lhs == rhs;
    }
};

template <>
struct IsEqual<boost::any> {
    static bool
    get(const boost::any &, const boost::any &)
    {
        // two boost::any cannot be safely compared to each other
        return false;
    }
};

template <typename KeyType, typename ValueType>
struct IsEqual<std::map<KeyType, ValueType>> {
    static bool
    get(const std::map<KeyType, ValueType> &lhs,
        const std::map<KeyType, ValueType> &rhs)
    {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
};

template <typename KeyType>
struct IsEqual<std::map<KeyType, boost::any>> {
    static bool
    get(const std::map<KeyType, boost::any> &,
        const std::map<KeyType, boost::any> &)
    {
        // two boost::any cannot be safely compared to each other
        return false;
    }
};

template <typename ValueType>
struct IsEqual<std::vector<ValueType>> {
    static bool
    get(const std::vector<ValueType> &lhs, const std::vector<ValueType> &rhs)
    {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
};

template <>
struct IsEqual<std::vector<boost::any>> {
    static bool
    get(const std::vector<boost::any> &, const std::vector<boost::any> &)
    {
        // two boost::any cannot be safely compared to each other
        return false;
    }
};

}  // namespace Settings
}  // namespace pajlada
