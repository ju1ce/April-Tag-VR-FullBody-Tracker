#pragma once

#include <ranges>
#include <string_view>

namespace serial
{

// template <>
// struct serial::Serial<T>
// {
//     static void Parse(auto& ctx, T& value) {}
//     static void Format(auto& ctx, const T& value) {}
// };
template <typename T>
struct Serial;

enum class Nest
{
    Expand,
    Compact
};

template <typename T, typename TValue>
concept SizedRangeOf = std::ranges::sized_range<T> && std::same_as<std::ranges::range_value_t<T>, TValue>;

template <typename T, typename TValue>
concept ReadableContext = requires(T& ctx, TValue val, std::string_view key) {
    ctx.template Read(val);
    { ctx.template ReadKey(key, val) } -> std::same_as<bool>;
    { ctx.ReadSeq() } -> SizedRangeOf<T>; };
template <typename T, typename TValue>
concept WritableContext = requires(T& ctx, const TValue val, std::string_view str, Nest nest) {
    ctx.template Write(val);
    ctx.template WriteKey(str, val);
    ctx.WriteComment(str);
    ctx.BeginMap(); ctx.BeginMap(nest);
    ctx.BeginSeq(); ctx.BeginSeq(nest);
    ctx.EndMap(); ctx.EndSeq(); };

template <typename T, typename TContext>
concept Parsable = requires(T val, TContext& context) { Serial<T>{}.Parse(context, val); };
template <typename T, typename TContext>
concept Formattable = requires(const T val, TContext& context) { Serial<T>{}.Format(context, val); };

template <typename T, typename TContext>
concept ParsableKey = requires(T val, TContext& context, const std::string_view key) {
     { Serial<T>{}.ParseKey(context, key, val) } -> std::same_as<bool>; };
template <typename T, typename TContext>
concept FormattableKey = requires(const T val, TContext& context, const std::string_view key) { Serial<T>{}.FormatKey(context, key, val); };

} // namespace serial
