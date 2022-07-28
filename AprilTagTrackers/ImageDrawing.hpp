#pragma once

#include "Config.hpp"

#include <opencv2/core.hpp>

void drawCalibration(
    cv::Mat& drawImg,
    const cv::Mat1d& cameraMatrix,
    const cv::Mat1d& distCoeffs,
    const cv::Mat1d& stdDeviationsIntrinsics,
    const std::vector<double>& perViewErrors,
    const std::vector<std::vector<cv::Point2f>>& allCharucoCorners,
    const std::vector<std::vector<int>>& allCharucoIds);

inline void drawCalibration(cv::Mat& drawImg, const CalibrationConfig& calib)
{
    drawCalibration(drawImg,
        calib.camMat,
        calib.distCoeffs,
        calib.stdDeviationsIntrinsics,
        calib.perViewErrors,
        calib.allCharucoCorners,
        calib.allCharucoIds);
}