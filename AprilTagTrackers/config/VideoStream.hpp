#pragma once

#include "utils/Reflectable.hpp"

#include <opencv2/core.hpp>

namespace cfg
{

// info to open a camera
struct Camera
{
    struct Extra
    {
        REFLECTABLE_BEGIN;
        REFLECTABLE_FIELD(bool, enabled) = false;
        REFLECTABLE_FIELD(double, exposure) = 0;
        REFLECTABLE_FIELD(double, autoExposure) = 0;
        REFLECTABLE_FIELD(double, gain) = 0;
        REFLECTABLE_END;
    };

    REFLECTABLE_BEGIN;
    REFLECTABLE_FIELD(std::string, address) = "0";
    REFLECTABLE_FIELD(cv::Size2i, resolution){0, 0};
    REFLECTABLE_FIELD(int, fps) = 0;
    REFLECTABLE_FIELD(int, api) = 0;
    REFLECTABLE_FIELD(int, rotateCl) = -1;
    REFLECTABLE_FIELD(bool, mirror) = false;
    REFLECTABLE_FIELD(bool, openDirectShowSettings) = false;
    REFLECTABLE_FIELD(Extra, extraSettings){};
    REFLECTABLE_END;
};

// config for a video stream that opens a camera
struct VideoStream
{
    REFLECTABLE_BEGIN;
    REFLECTABLE_FIELD(Camera, camera){};
    REFLECTABLE_FIELD(double, latency) = 0;
    REFLECTABLE_FIELD(double, quadDecimate) = 1;
    REFLECTABLE_FIELD(bool, circularWindow) = true;
    REFLECTABLE_FIELD(double, searchWindow) = 0.25;
    REFLECTABLE_END;
};

struct CameraCalib
{
    REFLECTABLE_BEGIN;
    REFLECTABLE_FIELD(cv::Mat, cameraMatrix){};
    REFLECTABLE_FIELD(cv::Mat, distortionCoeffs){};
    REFLECTABLE_FIELD(cv::Mat, stdDeviationsIntrinsics){};
    REFLECTABLE_FIELD(std::vector<double>, perViewErrors){};
    REFLECTABLE_FIELD(std::vector<std::vector<cv::Point2f>>, allCharucoCorners){};
    REFLECTABLE_FIELD(std::vector<std::vector<int>>, allCharucoIds){};
    REFLECTABLE_END;
};

} // namespace cfg
