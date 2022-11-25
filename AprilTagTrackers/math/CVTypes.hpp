#pragma once

#include "utils/StrongType.hpp"

#include <opencv2/aruco.hpp>
#include <opencv2/core/affine.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/quaternion.hpp>
#include <opencv2/core/types.hpp>

#include <vector>

namespace math
{

namespace detail
{

// clang-format off
template <typename T, int N>
struct CPoint;
template <typename T>
struct CPoint<T, 2> { using Type = cv::Point_<T>; };
template <typename T>
struct CPoint<T, 3> { using Type = cv::Point3_<T>; };
// clang-format on

} // namespace detail

template <typename T, int N>
using CPoint = typename detail::CPoint<T, N>::Type;

using RodriguesVec3d = utils::StrongType<cv::Vec3d, struct RodriguesVecTag_>;
using ArucoBoardSharedPtr = cv::Ptr<cv::aruco::Board>;
/// 4 corners of marker in ccw order, output from apriltag
using MarkerCorners2f = std::vector<cv::Point2f>;
/// 4 corners of marker in ccw order, most opencv aruco functions use this
using MarkerCorners3f = std::vector<cv::Point3f>;

constexpr inline int NUM_CORNERS = 4;

template <typename T>
inline constexpr int VecLength = cv::DataType<T>::channels;
template <typename T>
using VecValueType = typename cv::DataType<T>::channel_type;

template <typename T, int NLength>
concept VecLike = (VecLength<T> == NLength);

} // namespace math

using math::ArucoBoardSharedPtr;
using math::MarkerCorners2f;
using math::MarkerCorners3f;

/// alias vec access for x, y, z components.
/// writing val[X] + val[Y] is slightly easier to keep consistent than val[0] + val[1]
inline constexpr int X = 0;
inline constexpr int Y = 1;
inline constexpr int Z = 2;

template <typename T, int N>
struct std::hash<cv::Vec<T, N>> // NOLINT: specializing class template for user type
{
    constexpr std::size_t operator()(const cv::Vec<T, N>& vec) noexcept
    {
        std::size_t result = std::hash<T>()(vec[0]);
        for (std::size_t idx = 1; idx < N; ++idx)
        {
            result ^= std::hash<T>()(vec[idx]) << idx;
        }
        return result;
    }
};
