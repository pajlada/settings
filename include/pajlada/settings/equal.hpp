#pragma once

#ifndef PAJLADA_SETTINGS_NO_BOOST
#include <boost/any.hpp>
#endif

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

#ifndef PAJLADA_SETTINGS_NO_BOOST
template <>
struct IsEqual<boost::any> {
    static bool
    get(const boost::any &, const boost::any &)
    {
        // two boost::any cannot be safely compared to each other
        return false;
    }
};
#endif

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

#ifndef PAJLADA_SETTINGS_NO_BOOST
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
#endif

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

#ifndef PAJLADA_SETTINGS_NO_BOOST
template <>
struct IsEqual<std::vector<boost::any>> {
    static bool
    get(const std::vector<boost::any> &, const std::vector<boost::any> &)
    {
        // two boost::any cannot be safely compared to each other
        return false;
    }
};
#endif

}  // namespace Settings
}  // namespace pajlada
