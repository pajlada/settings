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

template <typename Type1, typename Type2>
struct IsEqual<std::pair<Type1, Type2>> {
    static bool
    get(const std::pair<Type1, Type2> &lhs, const std::pair<Type1, Type2> &rhs)
    {
        return IsEqual<Type1>::get(lhs.first, rhs.first) &&
               IsEqual<Type2>::get(lhs.second, rhs.second);
    }
};

#ifndef PAJLADA_SETTINGS_NO_BOOST
template <>
struct IsEqual<boost::any> {
    static bool
    get(const boost::any &lhs, const boost::any &rhs)
    {
        // two boost::any cannot be safely compared to each other, so we only consider them equal if both are empty
        return (lhs.empty() && rhs.empty());
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
    get(const std::map<KeyType, boost::any> &lhs,
        const std::map<KeyType, boost::any> &rhs)
    {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        for (const auto &p : lhs) {
            auto rit = rhs.find(p.first);

            if (rit == rhs.end()) {
                return false;
            }

            if (!IsEqual<boost::any>::get(p.second, rit->second)) {
                return false;
            }
        }

        return true;
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
    get(const std::vector<boost::any> &lhs, const std::vector<boost::any> &rhs)
    {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        auto lit = lhs.begin();
        auto rit = rhs.begin();

        for (; lit != lhs.begin(); ++lit, ++rit) {
            if (rit == rhs.end()) {
                return false;
            }

            if (!IsEqual<boost::any>::get(*lit, *rit)) {
                return false;
            }
        }

        return true;
    }
};
#endif

}  // namespace Settings
}  // namespace pajlada
