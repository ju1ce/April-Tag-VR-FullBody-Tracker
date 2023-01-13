#pragma once

#include "config/List.hpp"
#include "config/ManualCalib.hpp"
#include "config/TrackerUnit.hpp"
#include "config/Validated.hpp"
#include "config/VideoStream.hpp"
#include "serial/Comment.hpp"
#include "serial/Serializable.hpp"
#include "utils/Env.hpp"

#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>

#include <string>
#include <vector>

// Create definitions for each config file below

// Non-user editable calibration data, long lists of numbers.
// Potentially store this in a file.yaml.gz to reduce size,
//  and show that it is not user editable. FileStorage has this ability built in
class CalibrationConfig : public serial::Serializable<CalibrationConfig>
{
public:
    CalibrationConfig() : Serializable(utils::GetConfigDir() / "calib.yaml") {}

    REFLECTABLE_BEGIN;
    REFLECTABLE_FIELD(cfg::List<cfg::CameraCalib>, cameras){1};
    REFLECTABLE_FIELD(cfg::List<cfg::TrackerUnitCalib>, trackers){3};
    REFLECTABLE_END;
};

// user editable storage
class UserConfig : public serial::Serializable<UserConfig>
{
public:
    UserConfig() : Serializable(utils::GetConfigDir() / "config.yaml") {}

    REFLECTABLE_BEGIN;
    REFLECTABLE_FIELD(std::string, windowTitle);
    // Keep synced with Localization::LANG_CODE_MAP
    ATT_SERIAL_COMMENT("en, ru, zh-cn");
    REFLECTABLE_FIELD(std::string, langCode) = "en";
    REFLECTABLE_FIELD(int, trackerNum) = 3;
    REFLECTABLE_FIELD(cfg::Validated<double>, markerSize){5.0, cfg::GreaterEqual(0.01)};
    REFLECTABLE_FIELD(int, numOfPrevValues) = 5;
    REFLECTABLE_FIELD(bool, usePredictive) = true;
    REFLECTABLE_FIELD(bool, ignoreTracker0) = false;
    REFLECTABLE_FIELD(bool, coloredMarkers) = true;
    REFLECTABLE_FIELD(cfg::ManualCalib, manualCalib){};
    REFLECTABLE_FIELD(bool, chessboardCalib) = false;
    REFLECTABLE_FIELD(cfg::Validated<double>, smoothingFactor){0.5, cfg::Clamp(0.0, 1.0)};
    REFLECTABLE_FIELD(bool, circularMarkers) = false;
    REFLECTABLE_FIELD(cfg::Validated<double>, trackerCalibDistance){0.5, cfg::GreaterEqual(0.5)};
    /// TODO: change to not validated, gets set during calibration, to indicate if the user has done calibration
    REFLECTABLE_FIELD(cfg::Validated<int>, cameraCalibSamples){15, cfg::GreaterEqual(15)};
    REFLECTABLE_FIELD(bool, trackerCalibCenters) = false;
    REFLECTABLE_FIELD(cfg::Validated<double>, depthSmoothing){0, cfg::Clamp(0.0, 1.0)};
    REFLECTABLE_FIELD(float, additionalSmoothing) = 0;
    REFLECTABLE_FIELD(int, markerLibrary) = 0;
    /// TODO: if (value <= 0) value = 45;
    REFLECTABLE_FIELD(cfg::Validated<int>, markersPerTracker){45, cfg::GreaterEqual(1)};
    REFLECTABLE_FIELD(bool, disableOpenVrApi) = false;
    REFLECTABLE_FIELD(cfg::Validated<int>, apriltagThreadCount){4, cfg::Clamp(1, 32)};
    REFLECTABLE_FIELD(cfg::List<cfg::VideoStream>, videoStreams){1};
    REFLECTABLE_FIELD(cfg::List<cfg::TrackerUnit>, trackers){3};
    REFLECTABLE_FIELD(cfg::Validated<int>, detectorThreads){4, cfg::GreaterEqual(1)};
    REFLECTABLE_END;

    CalibrationConfig calib{};
};

class ArucoConfig : public serial::Serializable<ArucoConfig>
{
public:
    using ParamsPtr = cv::Ptr<cv::aruco::DetectorParameters>;

    ArucoConfig() : Serializable(utils::GetConfigDir() / "aruco.yaml")
    {
        auto p = cv::aruco::DetectorParameters::create();
        p->detectInvertedMarker = true;
        p->cornerRefinementMethod = cv::aruco::CORNER_REFINE_CONTOUR;
        params = p;
    }

    REFLECTABLE_BEGIN;
    REFLECTABLE_FIELD(ParamsPtr, params);
    REFLECTABLE_END;
};
