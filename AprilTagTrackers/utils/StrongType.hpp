#pragma once

#include "serial/Serial.hpp"

#include <concepts>
#include <ratio>
#include <utility>
#include <vector>

namespace utils
{

template <typename T>
concept UniqueTag = !requires { sizeof(T); };

template <typename T, UniqueTag>
struct StrongType
{
    using Type = T;

    /// list of strong types has the same memory layout as a list of the underlying type
    static std::vector<T>& ReinterpretVector(std::vector<StrongType>& list)
    {
        static_assert(sizeof(T) == sizeof(StrongType));
        return *reinterpret_cast<std::vector<T>*>(&list); // NOLINT: allow apis to take underlying type
    }

    constexpr StrongType() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : value()
    {
    }

    constexpr explicit StrongType(T&& val) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::is_trivially_move_constructible_v<T>
        : value(std::move(val))
    {
    }

    constexpr explicit StrongType(const T& val) noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires std::is_copy_constructible_v<T>
        : value(val)
    {
    }

    constexpr explicit operator T() const { return value; }

    template <typename TOther>
        requires(!std::same_as<T, TOther>)
    StrongType(TOther) = delete;

    template <typename TOther>
        requires(!std::same_as<T, TOther>)
    operator TOther() = delete;

    T value;
};

template <UniqueTag TTag>
using StrongBool = StrongType<bool, TTag>;

namespace detail
{

template <typename T>
concept IsRatio = std::same_as<T, std::ratio<T::num, T::den>>;

template <typename Rep, typename Ratio, typename ToRep, typename ToRatio>
constexpr ToRep RatioConvert(Rep&& value) noexcept
{
    using CRep = std::common_type_t<Rep, ToRep>;
    using CRatio = std::ratio_divide<Ratio, ToRatio>;
    if constexpr (CRatio::den == 1)
    {
        if constexpr (CRatio::num == 1)
        {
            return static_cast<ToRep>(std::forward<Rep>(value));
        }
        else
        {
            return static_cast<ToRep>(static_cast<CRep>(std::forward<Rep>(value)) *
                                      static_cast<CRep>(CRatio::num));
        }
    }
    else
    {
        if constexpr (CRatio::num == 1)
        {
            return static_cast<ToRep>(static_cast<CRep>(std::forward<Rep>(value)) /
                                      static_cast<CRep>(CRatio::den));
        }
        else
        {
            return static_cast<ToRep>((static_cast<CRep>(std::forward<Rep>(value)) *
                                       static_cast<CRep>(CRatio::num)) /
                                      static_cast<CRep>(CRatio::den));
        }
    }
}

} // namespace detail

template <typename TRep, detail::IsRatio TRatio, UniqueTag>
class StrongRatioType
{

public:
    using Rep = TRep;
    using Ratio = TRatio;

    // clang-format off
    template <std::same_as<TRep> T>
    constexpr explicit StrongRatioType(T rawValue) noexcept : value(std::move(rawValue)) {}
    constexpr StrongRatioType(StrongRatioType&& other) noexcept : value(std::move(other.value)) {}
    constexpr StrongRatioType(const StrongRatioType& other) noexcept : value(other.value) {}

    template <typename URep, typename URatio, typename UTag>
    constexpr explicit StrongRatioType(StrongRatioType<URep, URatio, UTag>&& other) noexcept
        : value(detail::RatioConvert<URep, URatio, TRep, TRatio>(std::move(other.value))) {}

    template <typename URep, typename URatio, typename UTag>
    constexpr explicit StrongRatioType(const StrongRatioType<URep, URatio, UTag>& other) noexcept
        : value(detail::RatioConvert<URep, URatio, TRep, TRatio>(other.value)) {}
    // clang-format on

    template <typename URep, typename URatio, typename UTag>
    constexpr StrongRatioType& operator*=(const StrongRatioType<URep, URatio, UTag>& rhs) noexcept
    {
        value *= detail::RatioConvert<URep, URatio, TRep, TRatio>(rhs.value);
        return *this;
    }
    template <typename URep, typename URatio, typename UTag>
    constexpr StrongRatioType& operator/=(const StrongRatioType<URep, URatio, UTag>& rhs) noexcept
    {
        value /= detail::RatioConvert<URep, URatio, TRep, TRatio>(rhs.value);
        return *this;
    }
    template <typename URep, typename URatio, typename UTag>
    constexpr StrongRatioType& operator+=(const StrongRatioType<URep, URatio, UTag>& rhs) noexcept
    {
        value += detail::RatioConvert<URep, URatio, TRep, TRatio>(rhs.value);
        return *this;
    }
    template <typename URep, typename URatio, typename UTag>
    constexpr StrongRatioType& operator-=(const StrongRatioType<URep, URatio, UTag>& rhs) noexcept
    {
        value -= detail::RatioConvert<URep, URatio, TRep, TRatio>(rhs.value);
        return *this;
    }

    /// public raw value, easy as possible to use this type without sacrificing type safety
    TRep value;
};

template <typename ARep, typename ARatio, typename BRep, typename BRatio, typename Tag>
constexpr StrongRatioType<ARep, ARatio, Tag> operator*(StrongRatioType<ARep, ARatio, Tag> rhs, const StrongRatioType<BRep, BRatio, Tag>& lhs) noexcept { return (rhs *= lhs); }
template <typename ARep, typename ARatio, typename BRep, typename BRatio, typename Tag>
constexpr StrongRatioType<ARep, ARatio, Tag> operator/(StrongRatioType<ARep, ARatio, Tag> rhs, const StrongRatioType<BRep, BRatio, Tag>& lhs) noexcept { return (rhs /= lhs); }
template <typename ARep, typename ARatio, typename BRep, typename BRatio, typename Tag>
constexpr StrongRatioType<ARep, ARatio, Tag> operator+(StrongRatioType<ARep, ARatio, Tag> rhs, const StrongRatioType<BRep, BRatio, Tag>& lhs) noexcept { return (rhs += lhs); }
template <typename ARep, typename ARatio, typename BRep, typename BRatio, typename Tag>
constexpr StrongRatioType<ARep, ARatio, Tag> operator-(StrongRatioType<ARep, ARatio, Tag> rhs, const StrongRatioType<BRep, BRatio, Tag>& lhs) noexcept { return (rhs -= lhs); }

} // namespace utils

template <typename T, typename TTag>
struct serial::Serial<utils::StrongType<T, TTag>>
{
    static void Parse(auto& ctx, utils::StrongType<T, TTag>& value) { ctx.Read(value.value); }
    static void Format(auto& ctx, const utils::StrongType<T, TTag>& value) { ctx.Write(value.value); }
};
template <typename TRep, typename TRatio, typename TTag>
struct serial::Serial<utils::StrongRatioType<TRep, TRatio, TTag>>
{
    static void Parse(auto& ctx, utils::StrongRatioType<TRep, TRatio, TTag>& value) { ctx.Read(value.value); }
    static void Format(auto& ctx, const utils::StrongRatioType<TRep, TRatio, TTag>& value) { ctx.Write(value.value); }
};
