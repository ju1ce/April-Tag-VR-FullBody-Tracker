#pragma once

#include "Config.hpp"

#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>

#include <string>
#include <vector>

const int APRILTAG_STANDARD = 0;
const int APRILTAG_CIRCULAR = 1;
const int ARUCO_4X4 = 2;
const int APRILTAG_COLOR = 3;
const int APRILTAG_CUSTOM29H10 = 4;

class AprilTagWrapper
{
public:
    explicit AprilTagWrapper(const UserConfig& user_config, const ArucoConfig& aruco_config);

    ~AprilTagWrapper();

    void convertToSingleChannel(const cv::Mat& src, cv::Mat& dst);

    void detectMarkers(
        const cv::Mat& frame,
        std::vector<std::vector<cv::Point2f>>* corners,
        std::vector<int>* ids,
        std::vector<cv::Point2f>* centers,
        std::vector<cv::Ptr<cv::aruco::Board>> trackers);

    std::vector<std::string> getTimeProfile();
    void drawTimeProfile(cv::Mat& image, const cv::Point& textOrigin);

private:
    cv::Ptr<cv::aruco::Dictionary> aruco_dictionary;
    struct apriltag_detector* const td;
    const UserConfig& user_config;
    const ArucoConfig& aruco_config;
};
