#pragma once

#define APP_VERSION "0.5.5"
#define DRIVER_VERSION "0.5.5"

#include "Quaternion.h"
#include "Serializable.h"
#include <opencv2/aruco.hpp>
#include <string>

// Define a reflectable field with optional validator,
// within a class that derives from FileStorageSerializable
#define FIELD(arg_type, arg_name, ...)                                \
public:                                                                 \
    REFLECTABLE_VISITOR(interface_, arg_type, arg_name, ##__VA_ARGS__); \
    arg_type arg_name

// Create definitions for each config file below

// User editable storage
class UserConfig : public FileStorageSerializable
{
public:
    UserConfig() : FileStorageSerializable("config.yaml")
    {
        Load();
    }

    FIELD(std::string, version) = APP_VERSION;
    FIELD(std::string, driver_version) = DRIVER_VERSION;

    FIELD(std::string, langId) = "en";
    FIELD(std::string, cameraAddr) = "0";
    FIELD(int, cameraApiPreference) = 0;
    FIELD(int, trackerNum) = 1;
    FIELD(double, markerSize,
            [](auto& value)
            { Clamp(value, 0., 1.); }) = 0.05;
    FIELD(int, numOfPrevValues) = 5;
    FIELD(double, quadDecimate) = 1;
    FIELD(double, searchWindow) = 0.25;
    FIELD(bool, usePredictive) = true;
    FIELD(int, calibrationTracker) = 0;
    FIELD(bool, chessboardCalib) = false;
    FIELD(bool, ignoreTracker0) = false;
    FIELD(bool, rotateCl) = false;
    FIELD(bool, rotateCounterCl) = false;
    FIELD(bool, coloredMarkers) = true;
    FIELD(double, calibOffsetX) = 0;
    FIELD(double, calibOffsetY) = 100;
    FIELD(double, calibOffsetZ) = 100;
    FIELD(double, calibOffsetA) = 100;
    FIELD(double, calibOffsetB) = 0;
    FIELD(double, calibOffsetC) = 0;
    FIELD(bool, circularWindow) = true;
    FIELD(double, smoothingFactor) = 0.5;
    FIELD(int, camFps) = 30;
    FIELD(int, camHeight) = 0;
    FIELD(int, camWidth) = 0;
    FIELD(bool, cameraSettings) = false;
    FIELD(double, camLatency) = 0;
    FIELD(bool, circularMarkers) = false;
    FIELD(double, trackerCalibDistance,
            [](auto& value)
            { if (value < 0.5) value = 0.5; }) = 0.5;
    FIELD(int, cameraCalibSamples,
            [](auto& value)
            { if (value < 15) value = 15; }) = 15;
    FIELD(bool, settingsParameters) = false;
    FIELD(double, cameraAutoexposure) = 0;
    FIELD(double, cameraExposure) = 0;
    FIELD(double, cameraGain) = 0;
    FIELD(bool, trackerCalibCenters) = false;
    FIELD(float, depthSmoothing) = 0;
    FIELD(float, additionalSmoothing) = 0;
    FIELD(int, markerLibrary) = 0;
    FIELD(int, markersPerTracker,
            [](auto& value)
            { if (value <= 0) value = 45; }) = 45;
    FIELD(int, languageSelection) = 0;
    FIELD(double, calibScale,
            [](auto& value)
            { if (value < 0.5) value = 1.0; }) = 1.0;
    FIELD(bool, disableOpenVrApi) = false;

private:
    template <typename T>
    static void Clamp(T& val, T min, T max)
    {
        val = val < min ? min : val > max ? max
                                          : val;
    }
};

// Non-user editable calibration data, long lists of numbers.
// Potentially store this in a file.yaml.gz to reduce size,
//  and show that it is not user editable. FileStorage has this ability built in
class CalibrationConfig : public FileStorageSerializable
{
public:
    CalibrationConfig() : FileStorageSerializable("calibration.yaml")
    {
        Load();
    }

    FIELD(cv::Mat, camMat);
    FIELD(cv::Mat, distCoeffs);
    FIELD(cv::Mat, stdDeviationsIntrinsics);
    FIELD(std::vector<double>, perViewErrors);
    FIELD(std::vector<std::vector<cv::Point2f>>, allCharucoCorners);
    FIELD(std::vector<std::vector<int>>, allCharucoIds);
    FIELD(std::vector<cv::Ptr<cv::aruco::Board>>, trackers);
    FIELD(cv::Mat, wtranslation);
    FIELD(Quaternion<double>, wrotation);
};

class ArucoConfig : public FileStorageSerializable
{
public:
    ArucoConfig() : FileStorageSerializable("aruco.yaml")
    {
        auto p = cv::aruco::DetectorParameters::create();
        p->detectInvertedMarker = true;
        p->cornerRefinementMethod = cv::aruco::CORNER_REFINE_CONTOUR;
        params = p;

        Load();
    }

    FIELD(cv::Ptr<cv::aruco::DetectorParameters>, params);
};

#undef FIELD