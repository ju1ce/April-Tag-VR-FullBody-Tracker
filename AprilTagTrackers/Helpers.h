#pragma once

#include "Debug.h"
#include "Quaternion.h"

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
template <typename T>
inline cv::Matx<T, 3, 3> EulerAnglesToRotationMatrix(cv::Vec<T, 3> theta)
{
    /// Calculate rotation about x axis
    cv::Matx<T, 3, 3> Rx{
        1, 0, 0,
        0, std::cos(theta[0]), -std::sin(theta[0]),
        0, std::sin(theta[0]), std::cos(theta[0])};
    /// Calculate rotation about y axis
    cv::Matx<T, 3, 3> Ry{
        std::cos(theta[1]), 0, std::sin(theta[1]),
        0, 1, 0,
        -std::sin(theta[1]), 0, std::cos(theta[1])};
    /// Calculate rotation about z axis
    cv::Matx<T, 3, 3> Rz{
        std::cos(theta[2]), -std::sin(theta[2]), 0,
        std::sin(theta[2]), std::cos(theta[2]), 0,
        0, 0, 1};
    // Combined rotation matrix
    return Rx * Ry * Rz;
}

template <typename T>
inline cv::Quat<T> RotationMatrixToQuat(const cv::Matx<T, 3, 3>& m)
{
    /// TODO: from opencv2/quaternion.inl.hpp
    // T S, w, x, y, z;
    // T trace = R(0, 0) + R(1, 1) + R(2, 2);
    // if (trace > 0)
    // {
    //     S = std::sqrt(trace + 1) * T(2);
    //     x = (R(1, 2) - R(2, 1)) / S;
    //     y = (R(2, 0) - R(0, 2)) / S;
    //     z = (R(0, 1) - R(1, 0)) / S;
    //     w = -T(0.25) * S;
    // }
    // else if (R(0, 0) > R(1, 1) && R(0, 0) > R(2, 2))
    // {

    //     S = std::sqrt(T(1.0) + R(0, 0) - R(1, 1) - R(2, 2)) * T(2);
    //     x = -T(0.25) * S;
    //     y = -(R(1, 0) + R(0, 1)) / S;
    //     z = -(R(0, 2) + R(2, 0)) / S;
    //     w = (R(1, 2) - R(2, 1)) / S;
    // }
    // else if (R(1, 1) > R(2, 2))
    // {
    //     S = std::sqrt(T(1.0) - R(0, 0) + R(1, 1) - R(2, 2)) * T(2);
    //     x = (R(0, 1) + R(1, 0)) / S;
    //     y = T(0.25) * S;
    //     z = (R(1, 2) + R(2, 1)) / S;
    //     w = (R(0, 2) - R(2, 0)) / S;
    // }
    // else
    // {
    //     S = std::sqrt(T(1.0) - R(0, 0) - R(1, 1) + R(2, 2)) * T(2);
    //     x = (R(0, 2) + R(2, 0)) / S;
    //     y = (R(1, 2) + R(2, 1)) / S;
    //     z = T(0.25) * S;
    //     w = -(R(0, 1) - R(1, 0)) / S;
    // }
    // return Quat<T> (w, x, y, z);

    /// TODO: This is the rotation matrix to quaternion in Connection::GetControllerPose,
    /// why is it different, does this produce the same result?
    // double qw = std::sqrt(std::fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
    // double qx = std::sqrt(std::fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
    // double qy = std::sqrt(std::fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
    // double qz = std::sqrt(std::fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
    // qx = std::copysign(qx, matrix.m[2][1] - matrix.m[1][2]);
    // qy = std::copysign(qy, matrix.m[0][2] - matrix.m[2][0]);
    // qz = std::copysign(qz, matrix.m[1][0] - matrix.m[0][1]);

    // Refactored version of mRot2Quat in Helpers.cpp, which is only used for calibration to station rotation.
    T q0 = (m(0, 0) + m(1, 1) + m(2, 2) + 1.0) / 4.0;
    T q1 = (m(0, 0) - m(1, 1) - m(2, 2) + 1.0) / 4.0;
    T q2 = (-m(0, 0) + m(1, 1) - m(2, 2) + 1.0) / 4.0;
    T q3 = (-m(0, 0) - m(1, 1) + m(2, 2) + 1.0) / 4.0;

    q0 = std::sqrt(std::max(q0, 0.0));
    q1 = std::sqrt(std::max(q1, 0.0));
    q2 = std::sqrt(std::max(q2, 0.0));
    q3 = std::sqrt(std::max(q3, 0.0));
    if (q0 >= q1 && q0 >= q2 && q0 >= q3)
    {
        // q0 *= +1.0f;
        q1 *= std::copysign(1.0, m(2, 1) - m(1, 2));
        q2 *= std::copysign(1.0, m(0, 2) - m(2, 0));
        q3 *= std::copysign(1.0, m(1, 0) - m(0, 1));
    }
    else if (q1 >= q0 && q1 >= q2 && q1 >= q3)
    {
        q0 *= std::copysign(1.0, m(2, 1) - m(1, 2));
        // q1 *= +1.0f;
        q2 *= std::copysign(1.0, m(1, 0) + m(0, 1));
        q3 *= std::copysign(1.0, m(0, 2) + m(2, 0));
    }
    else if (q2 >= q0 && q2 >= q1 && q2 >= q3)
    {
        q0 *= std::copysign(1.0, m(0, 2) - m(2, 0));
        q1 *= std::copysign(1.0, m(1, 0) + m(0, 1));
        // q2 *= +1.0f;
        q3 *= std::copysign(1.0, m(2, 1) + m(1, 2));
    }
    else if (q3 >= q0 && q3 >= q1 && q3 >= q2)
    {
        q0 *= std::copysign(1.0, m(1, 0) - m(0, 1));
        q1 *= std::copysign(1.0, m(2, 0) + m(0, 2));
        q2 *= std::copysign(1.0, m(2, 1) + m(1, 2));
        // q3 *= +1.0f;
    }
    else
    {
        ATFATAL("coding error.");
    }

    return cv::Quat<T>(q0, q1, q2, q3).normalize();
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
