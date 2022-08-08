#pragma once

#include "Comment.hpp"
#include "Deprecate.hpp"
#include "SemVer.h"
#include "utils/Assert.hpp"
#include "utils/Macros.hpp"
#include "utils/Reflectable.hpp"

#include <opencv2/aruco.hpp>
#include <opencv2/core/persistence.hpp>

#include <array>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace serial
{

using Reader = const cv::FileNode;
using Writer = cv::FileStorage;

/*
// add member functions
void ReadSelf(serial::Reader& reader);
// and/or
void WriteSelf(serial::Writer& writer);
*/

namespace detail
{

ATT_SFINAE_TYPE_TRAIT(HasReadSelf, typename, T,
    decltype(std::declval<T>().ReadSelf(std::declval<Reader&>())));

ATT_SFINAE_TYPE_TRAIT(HasWriteSelf, typename, T,
    decltype(std::declval<const T>().WriteSelf(std::declval<Writer&>())));

ATT_SFINAE_TYPE_TRAIT(IsResizable, typename, T,
    decltype(std::declval<T>().resize(std::declval<typename T::size_type>())));

// clang-format off
template <typename T> struct IsIterable : std::false_type {};
template <typename... Args> struct IsIterable<std::vector<Args...>> : std::true_type {};
template <typename T, size_t N> struct IsIterable<std::array<T, N>> : std::true_type {};

template <typename T> struct IsFileStorable : std::false_type {};
template <> struct IsFileStorable<bool> : std::true_type {};
template <> struct IsFileStorable<int> : std::true_type {};
template <> struct IsFileStorable<float> : std::true_type {};
template <> struct IsFileStorable<double> : std::true_type {};
template <> struct IsFileStorable<std::string> : std::true_type {};
template <> struct IsFileStorable<cv::Mat> : std::true_type {};
template<typename T, int N> struct IsFileStorable<cv::Vec<T, N>> : std::true_type {};
template<typename T, int M, int N> struct IsFileStorable<cv::Matx<T, M, N>> : std::true_type {};
template<typename T> struct IsFileStorable<cv::Point_<T>> : std::true_type {};
template<typename T> struct IsFileStorable<cv::Point3_<T>> : std::true_type {};
template<typename T> struct IsFileStorable<cv::Size_<T>> : std::true_type {};
// clang-format on

template <typename T>
inline constexpr bool HasReadSelfV = HasReadSelf<T>::value;
template <typename T>
inline constexpr bool HasWriteSelfV = HasWriteSelf<T>::value;
template <typename T>
inline constexpr bool IsResizableV = IsResizable<T>::value;
template <typename T>
inline constexpr bool IsIterableV = IsIterable<T>::value;
template <typename T>
inline constexpr bool IsFileStorableV = IsFileStorable<T>::value;

} // namespace detail

template <typename T>
inline void ReadReflectable(Reader& reader, T& value);
template <typename T>
inline void WriteReflectable(Writer& writer, const T& value);

template <typename T>
inline void ReadIterable(Reader& reader, T& value);
template <typename T>
inline void WriteIterable(Writer& writer, const T& value);

template <typename T,
    typename = std::enable_if_t<std::disjunction_v<
        detail::IsFileStorable<T>,
        Reflect::IsReflectable<T>,
        detail::HasReadSelf<T>,
        detail::IsIterable<T>>>>
inline void Read(Reader& reader, T& value)
{
    if constexpr (detail::HasReadSelfV<T>)
        value.ReadSelf(reader);
    else if constexpr (Reflect::IsReflectableV<T>)
        ReadReflectable(reader, value);
    else if constexpr (detail::IsIterableV<T>)
        ReadIterable(reader, value);
    else
        reader >> value;
}

template <typename T,
    typename = std::enable_if_t<std::disjunction_v<
        detail::IsFileStorable<T>,
        Reflect::IsReflectable<T>,
        detail::HasWriteSelf<T>,
        detail::IsIterable<T>>>>
inline void Write(Writer& writer, const T& value)
{
    if constexpr (detail::HasWriteSelfV<T>)
        value.WriteSelf(writer);
    else if constexpr (Reflect::IsReflectableV<T>)
    {
        if constexpr (Reflect::FieldCount<T> <= 4) writer << "{:";
        else writer << "{";
        WriteReflectable(writer, value);
        writer << "}";
    }
    else if constexpr (detail::IsIterableV<T>)
        WriteIterable(writer, value);
    else
        writer << value;
}

template <typename T>
inline void Read(Reader& reader, const std::string& key, T& value)
{
    Reader& elem = reader[key];
    if (!elem.empty()) Read(elem, value);
}
template <typename T>
inline void Write(Writer& writer, const std::string& key, const T& value)
{
    writer << key;
    Write(writer, value);
}

template <>
inline void Read(const Reader&, const std::string&, const Comment&)
{
    // nothing to read
}
template <>
inline void Write(Writer& writer, const std::string&, const Comment& value)
{
    writer.writeComment(value.str);
}

template <typename T>
inline void Write(Writer& writer, const std::string& key, const Deprecate<T>& value)
{
    // don't create the deprecated key
}
template <typename T>
inline void Read(Reader& reader, Deprecate<T>& value)
{
    Read(reader, value.copyTarget);
}

template <typename T>
inline void Read(Reader& reader, std::unique_ptr<T>& value)
{
    if (!value) value.reset(new T());
    Read(reader, *value);
}
template <typename T>
inline void Write(Writer& writer, const std::unique_ptr<T>& value)
{
    if (value) Write(writer, *value);
}

inline void Read(Reader& reader, SemVer& value)
{
    std::string temp;
    reader >> temp;
    value = SemVer::Parse(temp);
}
inline void Write(Writer& writer, const SemVer& value)
{
    writer << value.ToString();
}

template <>
inline void Read(Reader& reader, const std::string&, cv::Ptr<cv::aruco::DetectorParameters>& value)
{
    cv::aruco::DetectorParameters::readDetectorParameters(reader, value);
}
template <>
void Write(Writer& writer, const std::string&, const cv::Ptr<cv::aruco::DetectorParameters>& value);

inline void Read(Reader& reader, cv::Ptr<cv::aruco::Board>& value)
{
    // Resize might grow and initialize null cv::Ptr (alias of std::shared_ptr)
    /// TODO: this most likely breaks aruco tracking, use cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50) instead
    if (value.empty()) value = cv::makePtr<cv::aruco::Board>();
    // TODO: make our own aruco::Board, as we don't need to store the dictionary
    // Dictionary will be nullptr
    Read(reader, "ids", value->ids);
    Read(reader, "corners", value->objPoints);
}
inline void Write(Writer& writer, const cv::Ptr<cv::aruco::Board>& value)
{
    writer << "{";
    Write(writer, "ids", value->ids);
    Write(writer, "corners", value->objPoints);
    writer << "}";
}

template <typename T>
inline void ReadIterable(Reader& reader, T& value)
{
    cv::FileNodeIterator it = reader.begin();
    size_t readSize = it.remaining();
    if (readSize == 0) return;
    if constexpr (detail::IsResizableV<T>) value.resize(readSize);

    for (auto& elem : value)
    {
        Read(*it, elem);
        ++it;
    }
}
template <typename T>
inline void WriteIterable(Writer& writer, const T& value)
{
    using ValueT = typename T::value_type;
    if constexpr (std::disjunction_v<detail::IsIterable<ValueT>, std::is_integral<ValueT>>)
        writer << "[:";
    else writer << "[";

    for (const auto& elem : value)
        Write(writer, elem);

    writer << "]";
}

template <typename T>
inline void ReadReflectable(Reader& reader, T& value)
{
    Reflect::ForEach(value, [&reader](const char* name, auto& field)
        {
            Read(reader, std::string(name), field);
        });
}
template <typename T>
inline void WriteReflectable(Writer& writer, const T& value)
{
    Reflect::ForEach(value, [&writer](const char* name, const auto& field)
        {
            Write(writer, std::string(name), field);
        });
}

} // namespace serial
