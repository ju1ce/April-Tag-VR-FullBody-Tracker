#pragma once

#include <vector>

#include <opencv2/core.hpp>

typedef struct apriltag_detector apriltag_detector_t;
class Parameters;

class AprilTagWrapper
{
public:
    explicit AprilTagWrapper(const Parameters* parameters);

    ~AprilTagWrapper();

    void detectMarkers(
        const cv::Mat& frame,
        std::vector<std::vector<cv::Point2f> >* corners,
        std::vector<int>* ids,
        std::vector<cv::Point2f>* centers);

private:
    apriltag_detector_t*const td;
    const Parameters*const parameters;
};
