// Ceres Solver - A fast non-linear least squares minimizer
// Copyright 2015 Google Inc. All rights reserved.
// http://ceres-solver.org/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of Google Inc. nor the names of its contributors may be
//   used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Author: sameeragarwal@google.com (Sameer Agarwal)
//
// Templated struct implementing the camera model and residual
// computation for bundle adjustment used by Noah Snavely's Bundler
// SfM system. This is also the camera model/residual for the bundle
// adjustment problems in the BAL dataset. It is templated so that we
// can use Ceres's automatic differentiation to compute analytic
// jacobians.
//
// For details see: http://phototour.cs.washington.edu/bundler/
// and http://grail.cs.washington.edu/projects/bal/

#ifndef OPENCV_REPROJECTION_ERROR_HPP_
#define OPENCV_REPROJECTION_ERROR_HPP_

#include <iostream>

#include "ceres/rotation.h"

namespace ceres {
namespace examples {

// Templated pinhole camera model for used with Ceres.  The camera is
// parameterized using 15 parameters. 3 for rotation, 3 for
// translation, 2 for focal length, 2 for principal point, 3 for
// radial distortion and 2 for tangential distortion.
struct OpenCVReprojectionError {
  // (u, v): the position of the observation with respect to the image
  // center point.
  OpenCVReprojectionError(double observed_x, double observed_y,
                          double fx, double fy,
                          double cx, double cy,
                          double k1, double k2,
                          double p1, double p2,
                          double k3)
      : observed_x(observed_x), observed_y(observed_y)
      , fx(fx), fy(fy)
      , cx(cx), cy(cy)
      , k1(k1), k2(k2)
      , p1(p1), p2(p2)
      , k3(k3) {}

  template <typename T>
  bool operator()(const T* const camera,
                  const T* const point,
                  T* residuals) const {
    // camera[0,1,2] are the angle-axis rotation.
    T p[3];
    AngleAxisRotatePoint(camera, point, p);

    // camera[3,4,5] are the translation.
    p[0] += camera[3];
    p[1] += camera[4];
    p[2] += camera[5];

    const T xp = p[0] / p[2];
    const T yp = p[1] / p[2];

    // Calculate second, fourth and sixth order radial distortion.
    const T r2 = xp * xp + yp * yp;
    const T radial_distortion = 1.0 + r2 * (k1 + r2 * (k2 +  r2 * k3));

    // Calculate tangential distortion.
    const T xy = xp * yp;
    const T x2 = xp * xp;
    const T y2 = yp * yp;
    const T tangential_distortion_x = 2.0 * p1 * xy              +        p2 * (r2 + 2.0 * x2);
    const T tangential_distortion_y =       p1 * (r2 + 2.0 * y2) + 2.0  * p2 * xy;

    // Apply distortions
    const T xpp = xp * radial_distortion + tangential_distortion_x;
    const T ypp = yp * radial_distortion + tangential_distortion_y;

    // Compute final projected point position.
    const T predicted_x = fx * xpp + cx;
    const T predicted_y = fy * ypp + cy;

    // The error is the difference between the predicted and observed position.
    residuals[0] = predicted_x - observed_x;
    residuals[1] = predicted_y - observed_y;

    return true;
  }

#if 0
  template <typename T>
  bool operator()(const T* const camera,
                  const T* const point,
                  T* residuals) const {
    // camera[0,1,2] are the angle-axis rotation.
    T p[3];
    AngleAxisRotatePoint(camera, point, p);
    std::cout << "AngleAxisRotatePoint(camera, point, p)" << std::endl;
    std::cout << "camera[0, 1, 2] = [" << camera[0].a << ", "  << camera[1].a << ", "  << camera[2].a << "]" << std::endl;
    std::cout << "point[0, 1, 2] = [" << point[0].a << ", "  << point[1].a << ", "  << point[2].a << "]" << std::endl;
    std::cout << "p[0, 1, 2] = [" << p[0].a << ", "  << p[1].a << ", "  << p[2].a << "]" << std::endl;

    // camera[3,4,5] are the translation.
    p[0] += camera[3];
    std::cout << "p[0] += camera[3]" << std::endl;
    p[1] += camera[4];
    std::cout << "p[1] += camera[4]" << std::endl;
    p[2] += camera[5];
    std::cout << "p[2] += camera[5]" << std::endl;
    std::cout << "camera[3, 4, 5] = [" << camera[3].a << ", "  << camera[4].a << ", "  << camera[5].a << "]" << std::endl;

    const T xp = p[0] / p[2];
    std::cout << "const double xp = p[0] / p[2]" << std::endl;
    std::cout << "xp = " << xp.a << std::endl;

    const T yp = p[1] / p[2];
    std::cout << "const double yp = p[1] / p[2]" << std::endl;
    std::cout << "yp = " << yp.a << std::endl;

    // Calculate second, fourth and sixth order radial distortion.
    const T r2 = xp * xp + yp * yp;
    std::cout << "const double r2 = xp * xp + yp * yp" << std::endl;
    std::cout << "r2 = " << r2.a << std::endl;

    const T radial_distortion = 1.0 + r2 * (k1 + r2 * (k2 /*+  r2 * k3*/));
    std::cout << "const double radial_distortion = 1.0 + r2 * (k1 + r2 * (k2 /*+  r2 * k3*/))" << std::endl;
    std::cout << "k1 = " << k1 << std::endl;
    std::cout << "k2 = " << k2 << std::endl;
    std::cout << "k3 = " << k3 << std::endl;
    std::cout << "radial_distortion = " << radial_distortion.a << std::endl;

    // // Calculate tangential distortion.
    // const T xy = xp * yp;
    // const T x2 = xp * xp;
    // const T y2 = yp * yp;
    // const T tangential_distortion_x = 2.0 * p1 * xy              +        p2 * (r2 + 2.0 * x2);
    // const T tangential_distortion_y =       p1 * (r2 + 2.0 * y2) + 2.0  * p2 * xy;

    // Apply distortions
    const T xpp = xp * radial_distortion /*+ tangential_distortion_x*/;
    std::cout << "const double xpp = xp * radial_distortion /*+ tangential_distortion_x*/" << std::endl;
    std::cout << "xpp = " << xpp.a << std::endl;

    const T ypp = yp * radial_distortion /*+ tangential_distortion_y*/;
    std::cout << "const double ypp = yp * radial_distortion /*+ tangential_distortion_y*/" << std::endl;
    std::cout << "ypp = " << ypp.a << std::endl;

    // Compute final projected point position.
    const T predicted_x = fx * xpp + cx;
    std::cout << "const double predicted_x = fx * xpp + cx" << std::endl;
    std::cout << "predicted_x = " << predicted_x.a << std::endl;

    const T predicted_y = fy * ypp + cy;
    std::cout << "const double predicted_y = fy * ypp + cy" << std::endl;
    std::cout << "predicted_y = " << predicted_y.a << std::endl;

    // The error is the difference between the predicted and observed position.
    residuals[0] = predicted_x - observed_x;
    std::cout << "residuals[0] = predicted_x - observed_x" << std::endl;
    std::cout << "observed_x = " << observed_x << std::endl;
    std::cout << "residuals[0] = " << residuals[0].a << std::endl;

    residuals[1] = predicted_y - observed_y;
    std::cout << "residuals[1] = predicted_y - observed_y" << std::endl;
    std::cout << "observed_y = " << observed_y << std::endl;
    std::cout << "residuals[1] = " << residuals[1].a << std::endl;

    return true;
  }
#endif

#if 0
  bool operator()(const double* const camera,
                  const double* const point,
                  double* residuals) const {
    // camera[0,1,2] are the angle-axis rotation.
    double p[3];
    AngleAxisRotatePoint(camera, point, p);
    std::cout << "AngleAxisRotatePoint(camera, point, p)" << std::endl;
    std::cout << "camera[0, 1, 2] = [" << camera[0] << ", "  << camera[1] << ", "  << camera[2] << "]" << std::endl;
    std::cout << "point[0, 1, 2] = [" << point[0] << ", "  << point[1] << ", "  << point[2] << "]" << std::endl;
    std::cout << "p[0, 1, 2] = [" << p[0] << ", "  << p[1] << ", "  << p[2] << "]" << std::endl;

    // camera[3,4,5] are the translation.
    p[0] += camera[3];
    std::cout << "p[0] += camera[3]" << std::endl;
    p[1] += camera[4];
    std::cout << "p[1] += camera[4]" << std::endl;
    p[2] += camera[5];
    std::cout << "p[2] += camera[5]" << std::endl;
    std::cout << "camera[3, 4, 5] = [" << camera[3] << ", "  << camera[4] << ", "  << camera[5] << "]" << std::endl;
    std::cout << "p[0, 1, 2] = [" << p[0] << ", "  << p[1] << ", "  << p[2] << "]" << std::endl;

    const double xp = p[0] / p[2];
    std::cout << "const double xp = p[0] / p[2]" << std::endl;
    std::cout << "xp = " << xp << std::endl;

    const double yp = p[1] / p[2];
    std::cout << "const double yp = p[1] / p[2]" << std::endl;
    std::cout << "yp = " << yp << std::endl;

    // Calculate second, fourth and sixth order radial distortion.
    const double r2 = xp * xp + yp * yp;
    std::cout << "const double r2 = xp * xp + yp * yp" << std::endl;
    std::cout << "r2 = " << r2 << std::endl;

    const double radial_distortion = 1.0 + r2 * (k1 + r2 * (k2 /*+  r2 * k3*/));
    std::cout << "const double radial_distortion = 1.0 + r2 * (k1 + r2 * (k2 /*+  r2 * k3*/))" << std::endl;
    std::cout << "k1 = " << k1 << std::endl;
    std::cout << "k2 = " << k2 << std::endl;
    std::cout << "k3 = " << k3 << std::endl;
    std::cout << "radial_distortion = " << radial_distortion << std::endl;

    // // Calculate tangential distortion.
    // const double xy = xp * yp;
    // const double x2 = xp * xp;
    // const double y2 = yp * yp;
    // const double tangential_distortion_x = 2.0 * p1 * xy              +        p2 * (r2 + 2.0 * x2);
    // const double tangential_distortion_y =       p1 * (r2 + 2.0 * y2) + 2.0  * p2 * xy;

    // Apply distortions
    const double xpp = xp * radial_distortion /*+ tangential_distortion_x*/;
    std::cout << "const double xpp = xp * radial_distortion /*+ tangential_distortion_x*/" << std::endl;
    std::cout << "xpp = " << xpp << std::endl;

    const double ypp = yp * radial_distortion /*+ tangential_distortion_y*/;
    std::cout << "const double ypp = yp * radial_distortion /*+ tangential_distortion_y*/" << std::endl;
    std::cout << "ypp = " << ypp << std::endl;

    // Compute final projected point position.
    const double predicted_x = fx * xpp + cx;
    std::cout << "const double predicted_x = fx * xpp + cx" << std::endl;
    std::cout << "predicted_x = " << predicted_x << std::endl;

    const double predicted_y = fy * ypp + cy;
    std::cout << "const double predicted_y = fy * ypp + cy" << std::endl;
    std::cout << "predicted_y = " << predicted_y << std::endl;

    // The error is the difference between the predicted and observed position.
    residuals[0] = predicted_x - observed_x;
    std::cout << "residuals[0] = predicted_x - observed_x" << std::endl;
    std::cout << "residuals[0] = " << residuals[0] << std::endl;

    residuals[1] = predicted_y - observed_y;
    std::cout << "residuals[1] = predicted_y - observed_y" << std::endl;
    std::cout << "residuals[1] = " << residuals[1] << std::endl;

    return true;
  }
#endif

  // Factory to hide the construction of the CostFunction object from
  // the client code.
  static ceres::CostFunction* Create(const double observed_x,
                                     const double observed_y,
                                     const double fx, const double fy,
                                     const double cx, const double cy,
                                     const double k1, const double k2,
                                     const double p1, const double p2,
                                     const double k3) {
    return (new ceres::AutoDiffCostFunction<OpenCVReprojectionError, 2, 6, 3>(
        new OpenCVReprojectionError(observed_x, observed_y, fx, fy, cx, cy, k1, k2, p1, p2, k3)));
  }

  double observed_x;
  double observed_y;
  double fx, fy;
  double cx, cy;
  double k1, k2, p1, p2, k3;
};

}  // namespace examples
}  // namespace ceres

#endif  // OPENCV_REPROJECTION_ERROR_HPP_
