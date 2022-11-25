#pragma once

#include "config/VideoStream.hpp"
#include "CVTypes.hpp"
#include "Helpers.hpp"
#include "utils/Error.hpp"

#include <array>

namespace math
{

struct EstimatePoseSingleMarkersResult
{
    std::vector<cv::Vec3d> positions;
    std::vector<cv::Vec3d> rotations;
};

// This function receives the detected markers and returns their pose estimation respect to the camera individually. So for each marker, one rotation and translation vector is returned.
// The returned transformation is the one that transforms points from each marker coordinate system to the camera coordinate system.
// The marker corrdinate system is centered on the middle of the marker, with the Z axis perpendicular to the marker plane.
// The coordinates of the four corners of the marker in its own coordinate system are:
// (-markerLength/2, markerLength/2, 0), (markerLength/2, markerLength/2, 0),
// (markerLength/2, -markerLength/2, 0), (-markerLength/2, -markerLength/2, 0)
inline void EstimatePoseSingleMarkers(const std::vector<MarkerCorners2f>& corners,
                                      const double markerSize,
                                      const cfg::CameraCalib& camera,
                                      EstimatePoseSingleMarkersResult& result)
{
    result.positions.clear();
    result.rotations.clear();

    cv::aruco::estimatePoseSingleMarkers(
        corners,
        static_cast<float>(markerSize),
        camera.cameraMatrix, camera.distortionCoeffs,
        result.rotations, result.positions);
}

// This function receives the detected markers and returns the pose of a marker board composed by those markers.
// A Board of marker has a single world coordinate system which is defined by the board layout.
// The returned transformation is the one that transforms points from the board coordinate system to the camera coordinate system.
// Input markers that are not included in the board layout are ignored.
// The function returns the number of markers from the input employed for the board pose estimation.
// Note that returning a 0 means the pose has not been estimated.
inline std::tuple<RodrPose, int> EstimatePoseTracker(const std::vector<MarkerCorners2f>& corners,
                                                     const std::vector<int>& ids,
                                                     const ArucoBoardSharedPtr& board,
                                                     const cfg::CameraCalib& camera,
                                                     const bool usePredictive = false,
                                                     const RodrPose& predictiveGuess = {})
{
    RodrPose outPose = predictiveGuess;
    const int estimated = cv::aruco::estimatePoseBoard(
        corners, ids, board,
        camera.cameraMatrix, camera.distortionCoeffs,
        outPose.rotation.value, outPose.position,
        usePredictive);
    return {outPose, estimated};
}

inline cv::Size2i GetMatSize(const cv::Mat& mat) { return {mat.cols, mat.rows}; }

/// resize an image to a maximum width or height, while maintaining aspect ratio
inline cv::Size2i ConstrainSize(const cv::Size2i& size, int maxSize)
{
    if (size.width > size.height)
    {
        const double aspect = FloatDiv(maxSize, size.height);
        return {static_cast<int>(FloatMult(size.width, aspect)), maxSize};
    }
    const double aspect = FloatDiv(maxSize, size.width);
    return {maxSize, static_cast<int>(FloatMult(size.height, aspect))};
}
inline cv::Size2i ConstrainSize(const cv::Size2i& size, const cv::Size2i& fillSize)
{
    return ConstrainSize(size, std::min(fillSize.width, fillSize.height));
}

template <int NTo, typename T, int NFrom>
inline cv::Vec<T, NTo> ToVecOf(const cv::Vec<T, NFrom>& vec, T fill = 0)
{
    cv::Vec<T, NTo> result{vec.val};
    for (int i = NFrom; i < NTo; ++i)
    {
        result[i] = fill;
    }
    return result;
}
template <int NTo, typename T, int NFrom>
inline cv::Vec<T, NTo> ToVecOf(const cv::Matx<T, 1, NFrom>& vec, T fill = 0)
{
    cv::Vec<T, NTo> result{vec.val};
    for (int i = NFrom; i < NTo; ++i)
    {
        result[i] = fill;
    }
    return result;
}
template <int NTo, typename T, int NFrom>
inline cv::Vec<T, NTo> ToVecOf(const cv::Matx<T, NFrom, 1>& vec, T fill = 0)
{
    cv::Vec<T, NTo> result{vec.val};
    for (int i = NFrom; i < NTo; ++i)
    {
        result[i] = fill;
    }
    return result;
}

template <typename T, int N>
inline cv::Vec<T, N> ToVec(const cv::Matx<T, N, 1>& vec) { return {vec.val}; }
template <typename T, int N>
inline cv::Vec<T, N> ToVec(const cv::Matx<T, 1, N>& vec) { return {vec.val}; }
template <typename T>
inline cv::Vec<T, 2> ToVec(const cv::Point_<T>& vec) { return {vec.x, vec.y}; }
template <typename T>
inline cv::Vec<T, 3> ToVec(const cv::Point3_<T>& vec) { return {vec.x, vec.y, vec.z}; }
template <typename T>
inline cv::Vec<T, 3> ToVec(const cv::Size_<T>& vec) { return {vec.width, vec.height}; }

template <typename T, int N>
constexpr T& IndexVec(cv::Vec<T, N>& vec, int index)
{
    ATT_ASSERT(index >= 0 && index < N);
    return vec[index];
}
template <typename T>
constexpr T& IndexVec(cv::Point_<T>& point, int index)
{
    if (index == 0) return point.x;
    if (index == 1) return point.y;
    utils::Unreachable();
}
template <typename T>
constexpr T& IndexVec(cv::Point3_<T>& point, int index)
{
    if (index == 0) return point.x;
    if (index == 1) return point.y;
    if (index == 2) return point.z;
    utils::Unreachable();
}
template <typename T>
constexpr T& IndexVec(cv::Size_<T>& size, int index)
{
    if (index == 0) return size.width;
    if (index == 1) return size.height;
    utils::Unreachable();
}

template <typename T, int N>
constexpr const T& IndexVec(const cv::Vec<T, N>& vec, int index)
{
    ATT_ASSERT(index >= 0 && index < N);
    return vec[index];
}
template <typename T>
constexpr const T& IndexVec(const cv::Point_<T>& point, int index)
{
    if (index == 0) return point.x;
    if (index == 1) return point.y;
    utils::Unreachable();
}
template <typename T>
constexpr const T& IndexVec(const cv::Point3_<T>& point, int index)
{
    if (index == 0) return point.x;
    if (index == 1) return point.y;
    if (index == 2) return point.z;
    utils::Unreachable();
}
template <typename T>
constexpr const T& IndexVec(const cv::Size_<T>& size, int index)
{
    if (index == 0) return size.width;
    if (index == 1) return size.height;
    utils::Unreachable();
}

/// cv::Vec component-wise clamp
template <typename T>
constexpr T VecCompClamp(T vec, math::VecValueType<T> min, math::VecValueType<T> max)
{
    for (int i = 0; i < math::VecLength<T>; ++i)
    {
        IndexVec(vec, i) = std::clamp(IndexVec(vec, i), min, max);
    }
    return vec;
}
/// cv::Vec component-wise min
template <typename T>
constexpr T VecCompMin(T vec, math::VecValueType<T> val)
{
    for (int i = 0; i < math::VecLength<T>; ++i)
    {
        IndexVec(vec, i) = std::min(IndexVec(vec, i), val);
    }
    return vec;
}
/// cv::Vec component-wise max
template <typename T>
constexpr T VecCompMax(T vec, math::VecValueType<T> val)
{
    for (int i = 0; i < math::VecLength<T>; ++i)
    {
        IndexVec(vec, i) = std::max(IndexVec(vec, i), val);
    }
    return vec;
}

inline void CheckMarkerCorners(const MarkerCorners2f& corners)
{
    if (corners.size() != NUM_CORNERS) throw utils::Error("marker corners != 4");
}
inline void CheckMarkerCorners(const MarkerCorners3f& corners)
{
    if (corners.size() != NUM_CORNERS) throw utils::Error("marker corners != 4");
}

} // namespace math

using math::ConstrainSize;
using math::GetMatSize;
