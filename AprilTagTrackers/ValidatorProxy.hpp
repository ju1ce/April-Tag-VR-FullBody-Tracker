#pragma once

#include <opencv2/core/persistence.hpp>

#include <type_traits>

/// Controls access through setter and calls a validator on change.
template <typename T>
class ValidatorProxy
{
public:
    using Validator = void (*)(T& value);

    ValidatorProxy() = default;

    ValidatorProxy(T _value, Validator _validator)
        : value(std::forward<T>(_value)), validator(_validator) {}

    ValidatorProxy<T>& operator=(T&& rhs)
    {
        value = std::move(rhs);
        validator(value);
        return *this;
    }
    ValidatorProxy<T>& operator=(const T& rhs)
    {
        value = rhs;
        validator(value);
        return *this;
    }
    /// Returns true if the value was modified by the validator
    bool Set(T val)
    {
        value = std::move(val);
        validator(value);
        return value != val;
    }

    operator const T&() const { return value; }

    friend void Write(cv::FileStorage& fs, const ValidatorProxy<T>& proxy)
    {
        fs << proxy.value;
    }

    friend void Read(const cv::FileNode& fn, ValidatorProxy<T>& proxy)
    {
        fn >> proxy.value;
        proxy.validator(proxy.value);
    }

private:
    T value;
    Validator validator;
};
