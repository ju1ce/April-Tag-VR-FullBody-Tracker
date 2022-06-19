#pragma once

#include "Helpers.h"
#include "Quaternion.h"
#include "SemVer.h"
#include "Serializable.h"

#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>

#include <algorithm>
#include <string>

// Temporary alias
#define FIELD(a_type, a_name) \
    REFLECTABLE_FIELD(a_type, a_name)

class ManualCalib
{
private:
    /// Multiplier for posOffset when displayed in the gui
    static constexpr double POS_OFFSET_MULTI = 100;

public:
    /// Real representation of calib values, rather than what is displayed in gui and config
    struct Real
    {
        /// not multiplied by POS_OFFSET_MULTI
        cv::Vec3d posOffset;
        /// (pitch, yaw, roll) in radians;
        cv::Vec3d angleOffset;
        double scale;
    };

    Real GetAsReal()
    {
        return {
            posOffset * (1 / POS_OFFSET_MULTI),
            angleOffset * DEG_2_RAD,
            scale};
    }
    void SetFromReal(const Real& real)
    {
        posOffset = real.posOffset * POS_OFFSET_MULTI;
        angleOffset = real.angleOffset * RAD_2_DEG;
        scale = real.scale;
    }

    REFLECTABLE_BEGIN;
    /// stored multiplied by POS_OFFSET_MULTI
    FIELD(cv::Vec3d, posOffset);
    FS_COMMENT("(pitch, yaw, roll) in degrees.");
    FIELD(cv::Vec3d, angleOffset);
    FIELD(FS::Valid<double>, scale){
        1.0, [](auto& value)
        {
            value = std::clamp(value, 0.8, 1.2);
        }};
    REFLECTABLE_END;
};

// Create definitions for each config file below

// User editable storage
class UserConfig : public FS::Serializable<UserConfig>
{
public:
    UserConfig() : FS::Serializable<UserConfig>("config.yaml") {}

    REFLECTABLE_BEGIN;
    FIELD(SemVer, driver_version) = SemVer::Parse(REFLECTABLE_STRINGIZE(ATT_DRIVER_VERSION));

    FIELD(std::string, windowTitle);
    // Keep synced with Localization::LANG_CODE_MAP
    FS_COMMENT("en, zh-CN");
    FIELD(std::string, langCode) = "en";
    FIELD(std::string, cameraAddr) = "0";
    FIELD(int, cameraApiPreference) = 0;
    FIELD(int, trackerNum) = 3;
    FIELD(FS::Valid<double>, markerSize){
        5.0,
        [](auto& value)
        {
            if (value <= 0.) value = 0.1;
        }};
    FIELD(int, numOfPrevValues) = 5;
    FIELD(double, quadDecimate) = 1;
    FIELD(double, searchWindow) = 0.25;
    FIELD(bool, usePredictive) = true;
    FIELD(int, calibrationTracker) = 0;
    FIELD(bool, chessboardCalib) = false;
    FIELD(bool, ignoreTracker0) = false;
    FIELD(int, rotateCl) = -1;
    FIELD(bool, mirrorCam) = false;
    FIELD(bool, coloredMarkers) = true;
    FIELD(ManualCalib, manualCalib);
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
        {
            if (value < 0.5) value = 0.5;
        }};
    FIELD(FS::Valid<int>, cameraCalibSamples){
        15,
        [](auto& value)
        {
            if (value < 15) value = 15;
        }};
    FIELD(bool, settingsParameters) = false;
    FIELD(double, cameraAutoexposure) = 0;
    FIELD(double, cameraExposure) = 0;
    FIELD(double, cameraGain) = 0;
    FIELD(bool, trackerCalibCenters) = false;
    FIELD(FS::Valid<float>, depthSmoothing){
        0,
        [](auto& value)
        {
            value = std::clamp(value, 0.0f, 1.0f);
        }};
    FIELD(float, additionalSmoothing) = 0;
    FIELD(int, markerLibrary) = 0;
    FIELD(FS::Valid<int>, markersPerTracker){
        45,
        [](auto& value)
        {
            if (value <= 0) value = 45;
        }};
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
