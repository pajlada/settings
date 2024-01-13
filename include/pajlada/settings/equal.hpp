#pragma once

#include <any>
#include <map>
#include <vector>

namespace pajlada {

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

template <>
struct IsEqual<std::any> {
    static bool
    get(const std::any &lhs, const std::any &rhs)
    {
        // two std::any cannot be safely compared to each other, so we only consider them equal if both are empty
        return (!lhs.has_value() && !rhs.has_value());
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
struct IsEqual<std::map<KeyType, std::any>> {
    static bool
    get(const std::map<KeyType, std::any> &lhs,
        const std::map<KeyType, std::any> &rhs)
    {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        for (const auto &p : lhs) {
            auto rit = rhs.find(p.first);

            if (rit == rhs.end()) {
                return false;
            }

            if (!IsEqual<std::any>::get(p.second, rit->second)) {
                return false;
            }
        }

        return true;
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
struct IsEqual<std::vector<std::any>> {
    static bool
    get(const std::vector<std::any> &lhs, const std::vector<std::any> &rhs)
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

            if (!IsEqual<std::any>::get(*lit, *rit)) {
                return false;
            }
        }

        return true;
    }
};

}  // namespace pajlada
