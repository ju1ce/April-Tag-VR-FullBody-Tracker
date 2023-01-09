#ifndef CERES_EXAMPLES_POSITIONAL_ERROR_H_
#define CERES_EXAMPLES_POSITIONAL_ERROR_H_

namespace ceres {
namespace examples {

struct PositionalError {
  PositionalError(double anchor_x, double anchor_y, double anchor_z)
      : anchor_x(anchor_x), anchor_y(anchor_y), anchor_z(anchor_z) {}

  template <typename T>
  bool operator()(const T* const point,
                  T* residuals) const {
    // The error is the distance between current and anchor position.
    const T x = point[0] - anchor_x;
    const T y = point[1] - anchor_y;
    const T z = point[2] - anchor_z;

    const T x2 = x * x;
    const T y2 = y * y;
    const T z2 = z * z;

    residuals[0] = 1e6 * (x2 + y2 + z2);

    return true;
  }

  // Factory to hide the construction of the CostFunction object from
  // the client code.
  static ceres::CostFunction* Create(const double anchor_x,
                                     const double anchor_y,
                                     const double anchor_z) {
    return (new ceres::AutoDiffCostFunction<PositionalError, 1, 3>(
        new PositionalError(anchor_x, anchor_y, anchor_z)));
  }

  double anchor_x;
  double anchor_y;
  double anchor_z;
};


}  // namespace examples
}  // namespace ceres

#endif  // CERES_EXAMPLES_POSITIONAL_ERROR_H_
