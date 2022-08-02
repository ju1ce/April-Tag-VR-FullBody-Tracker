#pragma once

#include "SemVer.h"
#include "utils/Cross.hpp"
#include "utils/Macros.hpp"
#include "utils/Reflectable.hpp"

#include <opencv2/aruco.hpp>
#include <opencv2/core/persistence.hpp>

#include <iterator>
#include <string>
#include <type_traits>

namespace serial
{

namespace detail
{

ATT_SFINAE_TYPE_TRAIT(IsIterable, typename, T,
    decltype(std::begin(std::declval<T>())),
    decltype(std::end(std::declval<T>())));

template <typename T>
inline constexpr bool IsIterableV = typename IsIterable<T>::value;

ATT_SFINAE_TYPE_TRAIT(IsFileStorageReadable, typename, T,
    decltype(operator>>(
        std::declval<std::istream>(),
        std::declval<T>())));

template <typename T>
inline constexpr bool IsFileStorageReadableV = typename IsFileStorageWritable<T>::value;

ATT_SFINAE_TYPE_TRAIT(IsResizable, typename, T,
    decltype(std::declval<T>().resize(std::declval<typename T::size_type>())));

template <typename T>
inline constexpr bool IsResizableV = typename IsResizable<T>::value;

} // namespace detail

using Reader = cv::FileNode;

template <typename T>
void Read(const Reader& reader, T& value);
template <typename T>
void Read(const Reader& reader, const std::string& key, T& value);

template <typename T>
void ReadReflectable(const Reader& reader, T& value)
{
    Reflect::ForEach(value, [reader](const char* name, auto& field)
        {
            Read(reader, name, field);
        });
}

template <typename T>
void ReadIterable(const Reader& reader, T& value)
{
    cv::FileNodeIterator it = reader.begin();
    size_t size = it.remaining();

    if constexpr (detail::IsResizableV<T>) value.resize(size);

    for (auto& elem : value)
    {
        Read(reader, elem);
    }
}

template <typename T>
void Read(const Reader& reader, T& value)
{
    if constexpr (Reflect::IsReflectableV<T>)
        ReadReflectable(reader, value);
    else if constexpr (detail::IsIterableV<T>)
        ReadIterable(reader, value);
    else if constexpr (detail::IsFileStorageReadableV<T>)
        reader >> value;
    else
        static_assert(false, "missing overload ( " ATT_PRETTY_FUNCTION " ) for reader")
}

template <typename T>
void Read(const Reader& reader, const std::string& key, T& value)
{
    const Reader& elem = reader[key];
    if (!elem.empty()) Read(elem, value);
}

template <>
void Read(const Reader& reader, SemVer& value)
{
    std::string temp;
    reader >> temp;
    value = SemVer::Parse(temp);
}

template <>
void Read(const Reader& reader, cv::Ptr<cv::aruco::Board>& value)
{
    // Resize might grow and initialize null cv::Ptr (alias of std::shared_ptr)
    /// TODO: this most likely breaks aruco tracking, use cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50) instead
    if (value.empty()) value = cv::makePtr<cv::aruco::Board>();
    // TODO: make our own aruco::Board, as we don't need to store the dictionary
    // Dictionary will be nullptr
    reader["ids"] >> value->ids;
    reader["corners"] >> value->objPoints;
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
void serial::Write(serial::Writer& writer, const std::string& key, const Value& value);
void serial::Read(const serial::Reader& reader, const std::string& key, Value& value);
*/

} // namespace serial
