#pragma once

#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>

typedef struct apriltag_detector apriltag_detector_t;
class Parameters;

const int APRILTAG_STANDARD = 0;
const int APRILTAG_CIRCULAR = 1;
const int ARUCO_4X4 = 2;
const int APRILTAG_COLOR = 3;

class AprilTagWrapper
{
public:
    explicit AprilTagWrapper(const Parameters* parameters);

    ~AprilTagWrapper();

    void convertToSingleChannel(const cv::Mat& src, cv::Mat& dst);

    void detectMarkers(
        const cv::Mat& frame,
        std::vector<std::vector<cv::Point2f> >* corners,
        std::vector<int>* ids,
        std::vector<cv::Point2f>* centers,
        std::vector<cv::Ptr<cv::aruco::Board>> trackers
    );

    std::vector<std::string> getTimeProfile();
    void drawTimeProfile(cv::Mat& image, const cv::Point& textOrigin);

private:
    cv::Ptr<cv::aruco::Dictionary> aruco_dictionary;
    cv::Ptr<cv::aruco::DetectorParameters> aruco_params;
    apriltag_detector_t*const td;
    const Parameters*const parameters;
};
