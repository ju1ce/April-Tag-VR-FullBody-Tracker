#pragma once

#include "config/VideoStream.hpp"

#include <opencv2/core.hpp>

void drawCalibration(
    cv::Mat& drawImg,
    const cv::Mat1d& cameraMatrix,
    const cv::Mat1d& distCoeffs,
    const cv::Mat1d& stdDeviationsIntrinsics,
    const std::vector<double>& perViewErrors,
    const std::vector<std::vector<cv::Point2f>>& allCharucoCorners,
    const std::vector<std::vector<int>>& allCharucoIds);

inline void drawCalibration(cv::Mat& drawImg, const cfg::CameraCalibration& calib)
{
    drawCalibration(drawImg,
        calib.cameraMatrix,
        calib.distortionCoeffs,
        calib.stdDeviationsIntrinsics,
        calib.perViewErrors,
        calib.allCharucoCorners,
        calib.allCharucoIds);
}
