#pragma once

#include "Reader.hpp"
#include "Writer.hpp"

namespace serial
{

template <typename T>
class Deprecate
{
public:
    explicit Deprecate(T& _copyTarget) : copyTarget(_copyTarget) {}

    friend void Write<Deprecate>(Writer& writer, const std::string& key, const Deprecate& value);
    friend void Read<Deprecate>(const Reader& reader, Deprecate& value);

private:
    T& copyTarget;
};

template <typename T>
inline void Write<Deprecate<T>>(Writer& writer, const std::string& key, const Deprecate<T>& value)
{
    // don't create the deprecated key
}

template <typename T>
inline void Read<Deprecate<T>>(const Reader& reader, Deprecate<T>& value)
{
    Read(reader, value.copyTarget);
}

} // namespace serial
