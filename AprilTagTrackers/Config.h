#pragma once

#define APP_VERSION "0.5.5"
#define DRIVER_VERSION "0.5.5"

#include "Quaternion.h"
#include "Serializable.h"
#include <algorithm>
#include <opencv2/aruco.hpp>
#include <string>

// Define a reflectable field with optional validator,
// within a class that derives from FileStorageSerializable
#define FIELD(a_type, a_name) \
    REFLECTABLE_FIELD(a_type, a_name)

// Create definitions for each config file below

// User editable storage
class UserConfig : public FS::Serializable<UserConfig>
{
public:
    UserConfig() : FS::Serializable<UserConfig>("config.yaml") {}

    REFLECTABLE_BEGIN;
    FIELD(std::string, version) = APP_VERSION;
    FIELD(std::string, driver_version) = DRIVER_VERSION;

    FIELD(std::string, langCode) = "en";
    FIELD(std::string, cameraAddr) = "0";
    FIELD(int, cameraApiPreference) = 0;
    FIELD(int, trackerNum) = 1;
    FIELD(FS::Valid<double>, markerSize){
        0.05, [](auto& value)
        { value = std::clamp(value, 0., 1.); }};
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
    FIELD(FS::Valid<double>, trackerCalibDistance){
        0.5,
        [](auto& value)
        { if (value < 0.5) value = 0.5; }};
    FIELD(FS::Valid<int>, cameraCalibSamples){
        15,
        [](auto& value)
        { if (value < 15) value = 15; }};
    FIELD(bool, settingsParameters) = false;
    FIELD(double, cameraAutoexposure) = 0;
    FIELD(double, cameraExposure) = 0;
    FIELD(double, cameraGain) = 0;
    FIELD(bool, trackerCalibCenters) = false;
    FIELD(float, depthSmoothing) = 0;
    FIELD(float, additionalSmoothing) = 0;
    FIELD(int, markerLibrary) = 0;
    FIELD(FS::Valid<int>, markersPerTracker){
        45,
        [](auto& value)
        { if (value <= 0) value = 45; }};
    FIELD(FS::Valid<double>, calibScale){
        1.0,
        [](auto& value)
        { if (value < 0.5) value = 1.0; }};
    FIELD(bool, disableOpenVrApi) = false;
    REFLECTABLE_END;
};

// Non-user editable calibration data, long lists of numbers.
// Potentially store this in a file.yaml.gz to reduce size,
//  and show that it is not user editable. FileStorage has this ability built in
class CalibrationConfig : public FS::Serializable<CalibrationConfig>
{
public:
    CalibrationConfig() : FS::Serializable<CalibrationConfig>("calib.yaml") {}

    REFLECTABLE_BEGIN;
    FIELD(cv::Mat, camMat);
    FIELD(cv::Mat, distCoeffs);
    FIELD(cv::Mat, stdDeviationsIntrinsics);
    FIELD(std::vector<double>, perViewErrors);
    FIELD(std::vector<std::vector<cv::Point2f>>, allCharucoCorners);
    FIELD(std::vector<std::vector<int>>, allCharucoIds);
    FIELD(std::vector<cv::Ptr<cv::aruco::Board>>, trackers);
    FIELD(cv::Mat, wtranslation);
    FIELD(Quaternion<double>, wrotation);
    REFLECTABLE_END;
};

class ArucoConfig : public FS::Serializable<ArucoConfig>
{
public:
    ArucoConfig() : FS::Serializable<ArucoConfig>("aruco.yaml")
    {
        auto p = cv::aruco::DetectorParameters::create();
        p->detectInvertedMarker = true;
        p->cornerRefinementMethod = cv::aruco::CORNER_REFINE_CONTOUR;
        params = p;
    }

    REFLECTABLE_BEGIN;
    FIELD(cv::Ptr<cv::aruco::DetectorParameters>, params);
    REFLECTABLE_END;
};

#undef FIELD
