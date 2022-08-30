#pragma once

#include "serial/ReaderWriter.hpp"

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

    /// @param _validator void(T&)
    template <typename Fn>
    constexpr Validated(T _value, Fn&& _validator)
        : value(std::move(_value)), validator(std::forward<Fn>(_validator)) {}

    Validated& operator=(T rhs)
    {
        value = std::move(rhs);
        Validate();
        return *this;
    }

    operator const T&() const { return value; }
    const T* operator->() const { return &value; }

    const T& Get() const { return value; }

    void WriteSelf(serial::Writer& writer) const
    {
        serial::Write(writer, value);
    }
    void ReadSelf(serial::Reader& reader)
    {
        serial::Read(reader, value);
        Validate();
    }

private:
    void Validate() { validator(value); }

    T value;
    std::function<void(T&)> validator;
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
