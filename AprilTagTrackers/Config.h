#pragma once

#define APP_VERSION "0.5.5"
#define DRIVER_VERSION "0.5.5"

#include "Serializable.h"
#include "Quaternion.h"
#include <string>
#include <opencv2/aruco.hpp>

// Create definitions for each config file below

// User editable storage
class UserConfig : public FileStorageSerializable
{
public:
    UserConfig() : FileStorageSerializable("config.yaml")
    {
        Load();
    }

    template <typename T>
    static void clamp(T& val, T min, T max) {
        val = val < min ? min : val > max ? max : val;
    }

    S_FIELD(std::string, version) = APP_VERSION;
    S_FIELD(std::string, driver_version) = DRIVER_VERSION;

    S_FIELD(std::string, langId) = "en";
    S_FIELD(std::string, cameraAddr) = "0";
    S_FIELD(int, cameraApiPreference) = 0;
    S_FIELD(int, trackerNum) = 1;
    S_FIELD(double, markerSize,
        [](auto& value){ clamp(value, 0., 1.); }) = 0.05;
    S_FIELD(int, numOfPrevValues) = 5;
    S_FIELD(double, quadDecimate) = 1;
    S_FIELD(double, searchWindow) = 0.25;
    S_FIELD(bool, usePredictive) = true;
    S_FIELD(int, calibrationTracker) = 0;
    S_FIELD(bool, chessboardCalib) = false;
    S_FIELD(bool, ignoreTracker0) = false;
    S_FIELD(bool, rotateCl) = false;
    S_FIELD(bool, rotateCounterCl) = false;
    S_FIELD(bool, coloredMarkers) = true;
    S_FIELD(double, calibOffsetX) = 0;
    S_FIELD(double, calibOffsetY) = 100;
    S_FIELD(double, calibOffsetZ) = 100;
    S_FIELD(double, calibOffsetA) = 100;
    S_FIELD(double, calibOffsetB) = 0;
    S_FIELD(double, calibOffsetC) = 0;
    S_FIELD(bool, circularWindow) = true;
    S_FIELD(double, smoothingFactor) = 0.5;
    S_FIELD(int, camFps) = 30;
    S_FIELD(int, camHeight) = 0;
    S_FIELD(int, camWidth) = 0;
    S_FIELD(bool, cameraSettings) = false;
    S_FIELD(double, camLatency) = 0;
    S_FIELD(bool, circularMarkers) = false;
    S_FIELD(double, trackerCalibDistance,
        [](auto& value){ if (value < 0.5) value = 0.5; }) = 0.5;
    S_FIELD(int, cameraCalibSamples,
        [](auto& value){ if (value < 15) value = 15; }) = 15;
    S_FIELD(bool, settingsParameters) = false;
    S_FIELD(double, cameraAutoexposure) = 0;
    S_FIELD(double, cameraExposure) = 0;
    S_FIELD(double, cameraGain) = 0;
    S_FIELD(bool, trackerCalibCenters) = false;
    S_FIELD(float, depthSmoothing) = 0;
    S_FIELD(float, additionalSmoothing) = 0;
    S_FIELD(int, markerLibrary) = 0;
    S_FIELD(int, markersPerTracker,
        [](auto& value){ if (value <= 0) value = 45; }) = 45;
    S_FIELD(int, languageSelection) = 0;
    S_FIELD(double, calibScale,
        [](auto& value){ if (value < 0.5) value = 1.0; }) = 1.0;
    S_FIELD(bool, disableOpenVrApi) = false;
};

// Non-user editable calibration data, long lists of numbers.
// Potentially store this in a file.yaml.gz to reduce size,
//  and show that it is not user editable. FileStorage has this ability built in
class CalibrationConfig : public FileStorageSerializable {
public:
    CalibrationConfig() : FileStorageSerializable("calibration.yaml")
    {
        Load();
    }

    S_FIELD(cv::Mat, camMat);
    S_FIELD(cv::Mat, distCoeffs);
    S_FIELD(cv::Mat, stdDeviationsIntrinsics);
    S_FIELD(std::vector<double>, perViewErrors);
    S_FIELD(std::vector<std::vector<cv::Point2f>>, allCharucoCorners);
    S_FIELD(std::vector<std::vector<int>>, allCharucoIds);
    S_FIELD(std::vector<cv::Ptr<cv::aruco::Board>>, trackers);
    S_FIELD(cv::Mat, wtranslation);
    S_FIELD(Quaternion<double>, wrotation);
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

    S_FIELD(cv::Ptr<cv::aruco::DetectorParameters>, params);
};
