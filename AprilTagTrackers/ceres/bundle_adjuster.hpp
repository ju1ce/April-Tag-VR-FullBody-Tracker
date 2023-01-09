#ifndef CERES_EXAMPLES_BUNDLE_ADJUSTER_H_
#define CERES_EXAMPLES_BUNDLE_ADJUSTER_H_

namespace ceres::examples {

void SolveProblem(BALProblem &bal_problem);
void SolveProblem(const char* filename);

}  // namespace ceres::examples

#endif  // CERES_EXAMPLES_BAL_PROBLEM_H_
