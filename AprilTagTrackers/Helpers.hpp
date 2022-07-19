#pragma once

#include "Quaternion.hpp"
#include "utils/Assert.hpp"

#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/affine.hpp>
#include <opencv2/core/quaternion.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <cmath>

constexpr double PI = 3.14159265358979323846;
constexpr double RAD_2_DEG = 180.0 / PI;
constexpr double DEG_2_RAD = PI / 180.0;

void drawMarker(cv::Mat, std::vector<cv::Point2f>, cv::Scalar);
void transformMarkerSpace(std::vector<cv::Point3f>, cv::Vec3d, cv::Vec3d, cv::Vec3d, cv::Vec3d, std::vector<cv::Point3f>*);
void getMedianMarker(std::vector<std::vector<cv::Point3f>>, std::vector<cv::Point3f>*);
Quaternion<double> rodr2quat(double, double, double);
cv::Mat getSpaceCalib(cv::Vec3d, cv::Vec3d, double, double, double);
cv::Mat eulerAnglesToRotationMatrix(cv::Vec3f theta);
bool isRotationMatrix(cv::Mat& R);
cv::Vec3f rotationMatrixToEulerAngles(cv::Mat& R);
Quaternion<double> mRot2Quat(const cv::Mat& m);
cv::Vec3d quat2rodr(double qw, double qx, double qy, double qz);

template <typename T>
inline void RotateVecByQuat(cv::Vec<T, 3>& out_pos, const cv::Quat<T>& rot)
{
    cv::Quat<T> p{0, out_pos[0], out_pos[1], out_pos[2]};
    cv::Quat<T> r = rot * p * rot.inv();
    out_pos = cv::Vec<T, 3>{r.x, r.y, r.z};

    // // https://gamedev.stackexchange.com/a/50545
    // /// axis part of quaternion
    // cv::Vec<T, 3> axis{rot.x, rot.y, rot.z};
    // /// angle part of quaternion
    // T ang = rot.w;
    // // apply rotation to pos
    // pos = 2.0 * axis.dot(pos) * axis +
    //       (ang * ang - axis.dot(axis)) * pos +
    //       2.0 * ang * axis.cross(pos);
}

template <typename T>
inline T Length(T a)
{
    return std::abs(a);
}
template <typename T>
inline T Length(T a, T b)
{
    return std::sqrt(a * a + b * b);
}
template <typename T>
inline T Length(T a, T b, T c)
{
    return std::sqrt(a * a + b * b + c * c);
}
template <typename T>
inline T Length(cv::Vec<T, 3> a)
{
    return Length(a[0], a[1], a[2]);
}
template <typename T>
inline T Distance(T x1, T x2)
{
    return std::abs(x2 - x1);
}
template <typename T>
inline T Distance(T x1, T y1, T x2, T y2)
{
    return Length(x2 - x1, y2 - y1);
}
template <typename T>
inline T Distance(T x1, T y1, T z1, T x2, T y2, T z2)
{
    return Length(x2 - x1, y2 - y1, z2 - z1);
}
template <typename T>
inline T Distance(cv::Vec<T, 3> a, cv::Vec<T, 3> b)
{
    return Distance(a[0], a[1], a[2], b[0], b[1], b[2]);
}

/// Finds the rotation of a position from z forward
/// @returns Vec2(pitch, yaw) in radians
template <typename T>
inline cv::Vec<T, 2> EulerAnglesFromPos(cv::Vec<T, 3> pos, cv::Vec<T, 3> origin = cv::Vec<T, 3>::all(0))
{
    /// pos with an origin at 0
    cv::Vec<T, 3> p = pos - origin;
    /// offset from z forward around y axis
    double yaw = std::atan2(p[0], p[2]);
    /// offset from planar (xz) around x axis
    double pitch = std::atan2(p[1], Length(p[0], p[2]));
    return {pitch, yaw};
}

/// theta is pitch, yaw, roll in radians
/// YXZ
template <typename T>
inline cv::Matx<T, 3, 3> EulerAnglesToRotationMatrix(cv::Vec<T, 3> theta)
{
    /// Calculate rotation about y axis
    cv::Matx<T, 3, 3> Ry{
        std::cos(theta[1]), 0, std::sin(theta[1]),
        0, 1, 0,
        -std::sin(theta[1]), 0, std::cos(theta[1])};
    /// Calculate rotation about x axis
    cv::Matx<T, 3, 3> Rx{
        1, 0, 0,
        0, std::cos(theta[0]), -std::sin(theta[0]),
        0, std::sin(theta[0]), std::cos(theta[0])};
    /// Calculate rotation about z axis
    cv::Matx<T, 3, 3> Rz{
        std::cos(theta[2]), -std::sin(theta[2]), 0,
        std::sin(theta[2]), std::cos(theta[2]), 0,
        0, 0, 1};
    // Combined rotation matrix
    return Ry * Rx * Rz;
}

/// Transform from/to ovr coordinate system
/// by negating the [0]/x and [2]/z components.
template <typename T, int N>
inline void CoordTransformOVR(cv::Vec<T, N>& vec)
{
    vec[0] = -vec[0];
    if constexpr (N >= 3)
        vec[2] = -vec[2];
}
/// Transform from/to ovr coordinate system
/// by negating the x and z components.
template <typename T>
inline void CoordTransformOVR(cv::Quat<T>& quat)
{
    quat.x = -quat.x;
    quat.z = -quat.z;
}
