#pragma once

#include "SemVer.h"
#include "utils/Cross.hpp"
#include "utils/Macros.hpp"
#include "utils/Reflectable.hpp"

#include <opencv2/aruco.hpp>
#include <opencv2/core/persistence.hpp>

#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>

namespace serial
{

namespace detail
{

ATT_SFINAE_TYPE_TRAIT(IsConstIterable, typename, T,
    decltype(std::cbegin(std::declval<T>())),
    decltype(std::cend(std::declval<T>())));

template <typename T>
inline constexpr bool IsConstIterableV = typename IsConstIterable<T>::value;

ATT_SFINAE_TYPE_TRAIT(IsFileStorageWritable, typename, T,
    decltype(operator<<(
        std::declval<std::ostream>(),
        std::declval<T>())));

template <typename T>
inline constexpr bool IsFileStorageWritableV = typename IsFileStorageWritable<T>::value;

template <typename T>
inline constexpr bool IsStringV = std::is_base_of_v<std::basic_string<char>, T>;

} // namespace detail

using Writer = cv::FileStorage;

template <typename T>
void Write(Writer& writer, const T& value);
template <typename T>
void Write(Writer& writer, const std::string& key, const T& value);

template <typename T>
void WriteReflectable(Writer& writer, const T& value)
{
    if constexpr (Reflect::FieldCount<T> <= 4) writer << "{:";
    else writer << "{";

    Reflect::ForEach(value, [writer](const char* name, const auto& field)
        {
            Write(writer, name, field);
        });

    writer << "}";
}

template <typename T>
void WriteIterable(Writer& writer, const T& value)
{
    using ValueT = typename T::value_type;
    if constexpr (detail::ConstIterable<ValueT> || std::is_integral_v<ValueT>) writer << "[:";
    else writer << "[";

    for (const auto& elem : value)
        Write(writer, elem);

    writer.EndList();
}

template <typename T>
void Write(Writer& writer, const T& value)
{
    if constexpr (Reflect::IsReflectableV<T>)
        WriteReflectable(writer, value);
    else if constexpr (detail::IsConstIterableV<T>)
        WriteIterable(writer, value);
    else if constexpr (detail::IsFileStorageWritableV<T>)
        writer << value;
    else
        static_assert(false, "missing overload ( " ATT_PRETTY_FUNCTION " ) for writer");
}

template <typename T>
void Write(Writer& writer, const std::string& key, const T& value)
{
    writer << key;
    Write(writer, value);
}

template <>
void Write(Writer& writer, const std::string& value)
{
    writer << value;
}

template <>
void Write(Writer& writer, const SemVer& value)
{
    writer << value.ToString();
}

template <>
void Write(Writer& writer, const cv::Ptr<cv::aruco::Board>& value)
{
    writer << "{";
    writer << "ids" << value->ids;
    writer << "corners" << value->objPoints;
    writer << "}";
}

/*
// Define custom write/read methods for types

// Declare in header
template<> void serial::Write(serial::Writer& writer, const ValueT& value);
template<> void serial::Read(const serial::Reader& reader, ValueT& value);

// Implement
template<> void serial::Write(serial::Writer& writer, const ValueT& value) {}
template<> void serial::Read(const serial::Reader& reader, ValueT& value) {}

// Implement inline in header
template<> inline void serial::Write(serial::Writer& writer, const ValueT& value) {}
template<> inline void serial::Read(const serial::Reader& reader, ValueT& value) {}

// Friend in class to use private members, cannot implement here
friend void serial::Write<ValueT>(serial::Writer& writer, const ValueT& value);
friend void serial::Read<ValueT>(const serial::Reader& reader, ValueT& value);


// Alternate overloads with key string, to intercept key->value mapping,
// most likely not necessary unless creating custom serialization functionality
void serial::Write(serial::Writer& writer, const std::string& key, const ValueT& value);
void serial::Read(const serial::Reader& reader, const std::string& key, ValueT& value);
*/

} // namespace serial
