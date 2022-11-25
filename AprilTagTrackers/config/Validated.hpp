#pragma once

#include "serial/Serial.hpp"

#include <algorithm>
#include <functional>

namespace cfg
{

template <typename T>
class Validated
{
public:
    using IsProxyTag = void;
    using ValueType = T;

    friend struct serial::Serial<Validated>;

    /// @param validator void(T&)
    template <typename Fn>
    constexpr Validated(T value, Fn&& validator)
        : mValue(std::move(value)), mValidator(std::forward<Fn>(validator)) {}

    Validated& operator=(T rhs)
    {
        mValue = std::move(rhs);
        Validate();
        return *this;
    }

    operator const T&() const { return mValue; }
    const T* operator->() const { return &mValue; }

    const T& Get() const { return mValue; }

private:
    void Validate() { mValidator(mValue); }

    T mValue;
    std::function<void(T&)> mValidator;
};

template <typename T>
constexpr auto Clamp(const T& minInc, const T& maxInc)
{
    return [=](T& value)
    {
        value = std::clamp(value, minInc, maxInc);
    };
}

template <typename T>
constexpr auto GreaterEqual(const T& min)
{
    return [=](T& value)
    {
        value = std::max(value, min);
    };
}

template <typename T>
constexpr auto LessEqual(const T& max)
{
    return [=](T& value)
    {
        value = std::min(value, max);
    };
}

} // namespace cfg

template <typename T>
struct serial::Serial<cfg::Validated<T>>
{
    static void Parse(auto& ctx, cfg::Validated<T>& outValue)
    {
        ctx.Read(outValue.mValue);
        outValue.Validate();
    }
    static void Format(auto& ctx, const cfg::Validated<T>& value)
    {
        ctx.Write(value.mValue);
    }
};
