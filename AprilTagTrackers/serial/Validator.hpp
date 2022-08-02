#pragma once

#include <opencv2/core/persistence.hpp>

#include <type_traits>

namespace serial
{

/// Controls access through setter and calls a validator on change.
template <typename T>
class Validator
{
public:
    using ValidateFn = void (*)(T& value);

    Validator() = default;

    Validator(T _value, ValidateFn _validate)
        : value(std::move(_value)), validate(_validate) {}

    Validator& operator=(T rhs)
    {
        value = std::move(rhs);
        validate(value);
        return *this;
    }

    operator const T&() const { return value; }

    friend void Write<Validator>(Writer& writer, const Validator& proxy);
    friend void Read<Validator>(const Reader& reader, Validator& proxy);

private:
    T value{};
    ValidateFn validate = nullptr;
};

template <typename T>
inline void Write(Writer& writer, const Validator<T>& proxy)
{
    Write(writer, proxy.value);
}

template <typename T>
inline void Read(const Reader& reader, Validator<T>& proxy)
{
    Read(reader, proxy.value);
    proxy.validator(proxy.value);
}

} // namespace serial
