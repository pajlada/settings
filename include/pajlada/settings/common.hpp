#pragma once

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace pajlada {

namespace Settings {

enum class SettingOption : uint32_t {
    DoNotWriteToJSON = (1ULL << 1ULL),

    /// A remote setting is a setting that is never saved locally, nor registered locally with any callbacks or anything
    Remote = (1ULL << 2ULL),

    /// CompareBeforeSet compares the old & new value before updating the setting.
    /// This compares the JSON value, so a marshal & compare to the existing JSON value takes place
    CompareBeforeSet = (1ULL << 3ULL),

    Default = 0,
};

inline SettingOption
operator|(const SettingOption &lhs, const SettingOption &rhs)
{
    return static_cast<SettingOption>(
        (static_cast<uint64_t>(lhs) | static_cast<uint64_t>(rhs)));
}

inline SettingOption
operator&(const SettingOption &lhs, const SettingOption &rhs)
{
    return static_cast<SettingOption>(
        (static_cast<uint64_t>(lhs) & static_cast<uint64_t>(rhs)));
}

}  // namespace Settings

// specialize a type for all of the STL containers.
namespace is_stl_container_impl {

template <typename T>
struct is_stl_container : std::false_type {
};
template <typename T, std::size_t N>
struct is_stl_container<std::array<T, N>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::vector<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::deque<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::list<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::forward_list<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::set<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::multiset<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::map<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::multimap<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::unordered_set<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::unordered_multiset<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::unordered_map<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::unordered_multimap<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::stack<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::queue<Args...>> : std::true_type {
};
template <typename... Args>
struct is_stl_container<std::priority_queue<Args...>> : std::true_type {
};

}  // namespace is_stl_container_impl

// type trait to utilize the implementation type traits as well as decay the type
template <typename T>
struct is_stl_container {
    static constexpr bool const value =
        is_stl_container_impl::is_stl_container<std::decay_t<T>>::value;
};

}  // namespace pajlada
