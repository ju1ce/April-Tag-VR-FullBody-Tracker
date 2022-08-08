#pragma once

namespace serial
{

template <typename T>
struct Deprecate
{
    explicit Deprecate(T& _copyTarget) : copyTarget(_copyTarget) {}

    T& copyTarget;
};

} // namespace serial
