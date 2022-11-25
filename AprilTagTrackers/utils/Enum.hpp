#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <ranges>
#include <string_view>
#include <type_traits>

#define ATT_ENUM(p_name, ...)                                                                        \
    enum class p_name                                                                                \
    {                                                                                                \
        __VA_ARGS__                                                                                  \
    };                                                                                               \
    constexpr ::std::string_view ATTDetailReflectEnumStringized(::utils::renum::detail::Tag<p_name>) \
    {                                                                                                \
        return #__VA_ARGS__;                                                                         \
    }                                                                                                \
    constexpr auto ATTDetailReflectEnumValues(::utils::renum::detail::Tag<p_name>)                   \
    { /* // NOLINTNEXTLINE */                                                                        \
        const ::std::optional<::std::underlying_type_t<p_name>> __VA_ARGS__;                         \
        return ::std::to_array<::std::optional<::std::underlying_type_t<p_name>>>({__VA_ARGS__});    \
    }

namespace utils::renum
{

template <typename T>
concept Enum = std::is_enum_v<T>;

namespace detail
{

inline constexpr auto IsIdent = [](char c)
{ return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') ||
         c == '_'; };

using SubStringView = std::ranges::subrange<std::string_view::iterator>;

constexpr std::string_view::iterator FindOuterComma(const SubStringView str)
{
    int depth = 0;
    for (auto it = str.begin(); const char c : str)
    {
        if (c == ',' && depth == 0) return it + 1;
        if (c == '(')
        {
            ++depth;
        }
        else if (c == ')')
        {
            --depth;
        }
        ++it;
    }
    return str.end();
}

template <int Size>
constexpr std::array<std::string_view, Size> ParseEnumKeys(const std::string_view str)
{
    std::array<std::string_view, Size> members = {};
    auto it = str.begin();
    for (int i = 0; i < Size; ++i)
    {
        const auto ident = std::find_if(it, str.end(), IsIdent);
        it = std::find_if_not(ident + 1, str.end(), IsIdent);
        const auto offset = std::distance(str.begin(), ident);
        const auto length = std::distance(ident, it);
        members[i] = str.substr(offset, length);
        it = FindOuterComma({it, str.end()});
    }
    return members;
}

template <typename TInt, int Size>
constexpr std::array<TInt, Size> MapEnumValues(const std::array<std::optional<TInt>, Size>&& values)
{
    std::array<TInt, Size> result{};
    TInt previous = 0;
    for (std::size_t i = 0; i < Size; i++)
    {
        result[i] = values[i].value_or(previous);
        previous = result[i] + 1;
    }
    return result;
}

template <typename>
struct Tag
{
};

template <Enum TFirst, typename TSecond, typename... TRest>
    requires(sizeof...(TRest) % 2 == 0)
constexpr std::optional<TSecond> MatchPairs(TFirst value, TFirst first, TSecond second, TRest... rest) noexcept
{
    if (value == first) return std::forward<TSecond>(second);
    if constexpr (sizeof...(TRest) == 0)
    {
        return std::nullopt;
    }
    else
    {
        return MatchPairs<TFirst, TSecond>(rest...);
    }
}

} // namespace detail

// use adl to find enum meta data in other namespaces
constexpr std::string_view ATTDetailReflectEnumStringized(detail::Tag<void>);
constexpr auto ATTDetailReflectEnumValues(detail::Tag<void>);

template <typename T>
concept Reflectable = requires() {
                          ATTDetailReflectEnumStringized(detail::Tag<T>{});
                          ATTDetailReflectEnumValues(detail::Tag<T>{});
                      };

template <Reflectable T>
inline constexpr std::string_view Stringized = ATTDetailReflectEnumStringized(detail::Tag<T>{});

template <Reflectable T>
inline constexpr std::size_t Size = ATTDetailReflectEnumValues(detail::Tag<T>{}).size();

template <Reflectable T>
inline constexpr std::array<std::underlying_type_t<T>, Size<T>> Values =
    detail::MapEnumValues<std::underlying_type_t<T>, Size<T>>(ATTDetailReflectEnumValues(detail::Tag<T>{}));

template <Reflectable T>
inline constexpr std::array<std::string_view, Size<T>> Keys =
    detail::ParseEnumKeys<Size<T>>(Stringized<T>);

template <Reflectable T>
constexpr bool IsSequential()
{
    const auto offset = Values<T>[0];
    for (std::size_t i = 1; i < Size<T>; ++i)
    {
        const auto index = static_cast<std::underlying_type_t<T>>(i);
        if (Values<T>[i] != (index + offset)) return false;
    }
    return true;
}

template <typename T>
concept Sequential = IsSequential<T>();

template <Reflectable T>
constexpr bool IsUnique()
{
    auto values = Values<T>;
    std::sort(values.begin(), values.end());
    auto lastIt = std::unique(values.begin(), values.end());
    return lastIt == values.end();
}

template <typename T>
concept Unique = Sequential<T> || IsUnique<T>();

template <Reflectable T>
constexpr std::optional<T> FromString(std::string_view name)
{
    auto keyIt = std::find(Keys<T>.begin(), Keys<T>.end(), name);
    if (keyIt == Keys<T>.end()) return std::nullopt;
    const auto valueIt = Values<T>.begin() + std::distance(Keys<T>.begin(), keyIt);
    return static_cast<T>(*valueIt);
}

template <Sequential T>
constexpr std::string_view ToString(T val)
{
    constexpr auto offset = static_cast<std::int64_t>(Values<T>[0]);
    const auto valRep = static_cast<std::int64_t>(val);
    return Keys<T>.at(valRep - offset);
}

template <Enum T>
constexpr bool IsAnyOf(T value, std::same_as<T> auto... values) noexcept
{
    return ((value == values) || ...);
}

template <typename TResult, Enum TEnum, typename... TArgs>
constexpr std::optional<TResult> Map(TEnum value, TArgs&&... mappingPairs)
{
    return detail::MatchPairs<TEnum, TResult>(value, std::forward<TArgs>(mappingPairs)...);
}

} // namespace utils::renum

namespace renum = utils::renum; // NOLINT
