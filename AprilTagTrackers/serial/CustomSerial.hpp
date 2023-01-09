#pragma once

#include "FileStorage.hpp"
#include "SemVer.h"
#include "Serial.hpp"
#include "utils/Error.hpp"
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

namespace detail
{

// clang-format off
template <typename> inline constexpr bool IsIterable = false;
template <typename T, typename... TArgs> inline constexpr bool IsIterable<std::vector<T, TArgs...>> = true;
template <typename T, std::size_t N> inline constexpr bool IsIterable<std::array<T, N>> = true;
// clang-format on

template <typename T>
concept Resizable = requires(T val, std::size_t n) { val.resize(n); };

void WriteReflectable(auto& ctx, const auto& value)
{
    Reflect::ForEach(value, [&](std::string_view subKey, const auto& subValue)
                     { ctx.WriteKey(subKey, subValue); });
}

void ReadReflectable(auto& ctx, auto& value)
{
    Reflect::ForEach(value, [&](std::string_view subKey, auto& subValue)
                     { ctx.ReadKey(subKey, subValue); });
}

} // namespace detail

template <typename T>
    requires Reflect::IsReflectableV<T>
struct Serial<T>
{
    static void Parse(auto& ctx, T& value)
    {
        detail::ReadReflectable(ctx, value);
    }
    static void Format(auto& ctx, const T& value)
    {
        ctx.BeginMap();
        detail::WriteReflectable(ctx, value);
        ctx.EndMap();
    }
};

template <typename T>
    requires detail::IsIterable<T>
struct Serial<T>
{
    static void Parse(auto& ctx, T& list)
    {
        std::ranges::sized_range auto range = ctx.ReadSeq();
        const std::size_t readSize = std::size(range);
        if (readSize == 0) return;
        if constexpr (detail::Resizable<T>) list.resize(readSize);
        if (list.size() != readSize) throw utils::MakeError("parsed sized ", readSize, " != ", " storage size ", list.size());

        auto it = range.begin();
        for (auto& value : list)
        {
            (*it).Read(value);
            ++it;
        }
    }
    static void Format(auto& ctx, const T& list)
    {
        using ValueT = typename T::value_type;
        ctx.BeginSeq(std::is_integral_v<ValueT> ? Nest::Compact : Nest::Expand);
        for (const auto& value : list)
        {
            ctx.Write(value);
        }
        ctx.EndSeq();
    }
};

template <typename T>
struct Serial<std::unique_ptr<T>>
{
    static void Parse(auto& ctx, std::unique_ptr<T>& value)
    {
        if (!value) value = std::make_unique<T>();
        ctx.Read(*value);
    }
    static void Format(auto& ctx, const std::unique_ptr<T>& value)
    {
        if (value) ctx.Write(*value);
    }
};

template <>
struct Serial<SemVer>
{
    static void Parse(auto& ctx, SemVer& outValue)
    {
        std::string temp;
        ctx.Read(temp);
        outValue = SemVer::Parse(temp);
    }
    static void Format(auto& ctx, const SemVer& value)
    {
        std::string temp = value.ToString();
        ctx.AdvanceTo(std::copy(temp.begin(), temp.end(), ctx.Out()));

        ctx.Write(value.ToString());
    }
};

template <>
struct Serial<cv::Ptr<cv::aruco::DetectorParameters>>
{
    static void Parse(FileStorageReader& ctx, cv::Ptr<cv::aruco::DetectorParameters>& value)
    {
        value->readDetectorParameters(ctx.GetReader());
    }
    static void Format(FileStorageWriter& ctx, const cv::Ptr<cv::aruco::DetectorParameters>& value);
};

template <>
struct Serial<cv::Ptr<cv::aruco::Board>>
{
    static void Parse(FileStorageReader& ctx, cv::Ptr<cv::aruco::Board>& value)
    {
        // Resize might grow and initialize null cv::Ptr (alias of std::shared_ptr)
        /// TODO: this most likely breaks aruco tracking, use cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50) instead
        if (value.empty()) value = cv::makePtr<cv::aruco::Board>();
        // TODO: make our own aruco::Board, as we don't need to store the dictionary
        // Dictionary will be nullptr
        ctx.ReadKey("ids", value->ids);
        ctx.ReadKey("corners", value->objPoints);
    }
    static void Format(FileStorageWriter& ctx, const cv::Ptr<cv::aruco::Board>& value)
    {
        ctx.BeginMap();
        ctx.WriteKey("ids", value->ids);
        ctx.WriteKey("corners", value->objPoints);
        ctx.EndMap();
    }
};

} // namespace serial
