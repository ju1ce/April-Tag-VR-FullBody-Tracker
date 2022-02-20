#pragma once

#include "Serializable.h"
#include "opencv2/aruco.hpp"
#include "opencv2/core/persistence.hpp"
#include <algorithm>
#include <string>
#include "Quaternion.h"

class FileStorageSerializer : public Serializer<const cv::FileNode, cv::FileStorage>
{
    FileStorageSerializer(const std::string& file_path)
        : file_path(file_path) {}

    bool Save() {
        cv::FileStorage fs(file_path, cv::FileStorage::READ);
        if (!fs.isOpened()) return false;
        assert(fs.getFormat() == cv::FileStorage::FORMAT_YAML);
        Serialize(fs);
        fs.release();
        return true;
    }

    void Load() {
        cv::FileStorage fs(file_path, cv::FileStorage::WRITE);
        if (!fs.isOpened()) return false;
        assert(fs.getFormat() == cv::FileStorage::FORMAT_YAML);
        Deserialize(fs);
        fs.release();
        return true;
    }

protected:
    virtual void Serialize(OS &os) const
    {
        for (const auto& fd : all_interfaces)
        {
            os << fd->name;
            fd->Serialize(os);
        }
    }

    virtual void Deserialize(IS &is)
    {
        for (const auto& fd : all_interfaces)
        {
            fd->Deserialize(is[fd->name]);
        }
    }

    template<typename FieldData>
    void DeserializeField(IS &is, const FieldData& fd) /* overload */
    {
        if (is.empty()) return;
        typename FieldData::FieldType temp;
        is >> temp;
        fd.set(std::move(temp));
    }

private:
    const std::string file_path;
};

// Don't assign a name to each field
class FileStorageFlatSerializer : public FileStorageSerializer
{
    void Serialize(OS &os) const override
    {
        for (const auto& fd : all_interfaces)
            fd->Serialize(os);
    }
    void Deserialize(IS &is) override
    {
        for (const auto& fd : all_interfaces)
            fd->Deserialize(is);
    }
};

#define CV_CUSTOM_SRL_TYPE(arg_type) \
    cv::internal::WriteStructContext _wsc(fs, cv::String(), cv::FileNode::MAP + cv::FileNode::FLOW, arg_type)

static cv::FileStorage &operator<<(cv::FileStorage &fs, const cv::Ptr<cv::aruco::Board> &s)
{
    CV_CUSTOM_SRL_TYPE("cv::aruco::Boards");
    fs << "ids" << s->ids;
    fs << "objPoints" << s->objPoints;
    return fs;
}
static void operator>>(const cv::FileNode &fn, cv::Ptr<cv::aruco::Board> &s)
{
    fn["ids"] >> s->ids;
    fn["objPoints"] >> s->objPoints;
}

template<typename T>
static cv::FileStorage &operator<<(cv::FileStorage &fs, const Quaternion<T>& q)
{
    cv::Quat<T> temp(q.w, q.x, q.y, q.z);
    fs << temp;
    return fs;
}
template<typename T>
static void operator>>(const cv::FileNode &fn, Quaternion<T>& q)
{
    cv::Quat<T> temp;
    fn >> temp;
    q = Quaternion<T>(temp.w, temp.x, temp.y, temp.z);
}

// OpenCV dosnt have an implementation for storing its own aruco config file...
static cv::FileStorage &operator<<(cv::FileStorage &fs, const cv::Ptr<cv::aruco::DetectorParameters> &s)
{
    fs << "adaptiveThreshWinSizeMin" << s->adaptiveThreshWinSizeMin;
    fs << "adaptiveThreshWinSizeMax" << s->adaptiveThreshWinSizeMax;
    fs << "adaptiveThreshWinSizeStep" << s->adaptiveThreshWinSizeStep;
    fs << "adaptiveThreshConstant" << s->adaptiveThreshConstant;
    fs << "minMarkerPerimeterRate" << s->minMarkerPerimeterRate;
    fs << "maxMarkerPerimeterRate" << s->maxMarkerPerimeterRate;
    fs << "polygonalApproxAccuracyRate" << s->polygonalApproxAccuracyRate;
    fs << "minCornerDistanceRate" << s->minCornerDistanceRate;
    fs << "minDistanceToBorder" << s->minDistanceToBorder;
    fs << "minMarkerDistanceRate" << s->minMarkerDistanceRate;
    fs << "cornerRefinementMethod" << s->cornerRefinementMethod;
    fs << "cornerRefinementWinSize" << s->cornerRefinementWinSize;
    fs << "cornerRefinementMaxIterations" << s->cornerRefinementMaxIterations;
    fs << "cornerRefinementMinAccuracy" << s->cornerRefinementMinAccuracy;
    fs << "markerBorderBits" << s->markerBorderBits;
    fs << "perspectiveRemovePixelPerCell" << s->perspectiveRemovePixelPerCell;
    fs << "perspectiveRemoveIgnoredMarginPerCell" << s->perspectiveRemoveIgnoredMarginPerCell;
    fs << "maxErroneousBitsInBorderRate" << s->maxErroneousBitsInBorderRate;
    fs << "minOtsuStdDev" << s->minOtsuStdDev;
    fs << "errorCorrectionRate" << s->errorCorrectionRate;

    // April :: User-configurable parameters.
    fs << "aprilTagQuadDecimate" << s->aprilTagQuadDecimate;
    fs << "aprilTagQuadSigma" << s->aprilTagQuadSigma;

    // April :: Internal variables
    fs << "aprilTagMinClusterPixels" << s->aprilTagMinClusterPixels;
    fs << "aprilTagMaxNmaxima" << s->aprilTagMaxNmaxima;
    fs << "aprilTagCriticalRad" << s->aprilTagCriticalRad;
    fs << "aprilTagMaxLineFitMse" << s->aprilTagMaxLineFitMse;
    fs << "aprilTagMinWhiteBlackDiff" << s->aprilTagMinWhiteBlackDiff;
    fs << "aprilTagDeglitch" << s->aprilTagDeglitch;

    // to detect white (inverted) markers
    fs << "detectInvertedMarker" << s->detectInvertedMarker;
    return fs;
}

static void operator>>(const cv::FileNode &fn, cv::Ptr<cv::aruco::DetectorParameters> &s)
{
    cv::aruco::DetectorParameters::readDetectorParameters(fn, s);
}

// Create definitions for each config file below

// User editable storage
SRL_CLASS(UserConfigStorage, FileStorageSerializer,
    UserConfigStorage()
        : FileStorageSerializer("config.yaml")
    {
        Load();
    }

    template<typename T>
    static void clamp(T& val, T min, T max) {
        val = val < min ? min : val > max ? max : val;
    }

    SRL_FIELD(std::string, cameraAddr) = "0";
    SRL_FIELD(int, cameraApiPreference) = 0;
    SRL_FIELD(int, trackerNum) = 1;
    SRL_FIELD_V(double, markerSize,
        { clamp(value, 0., 1.); }) = 0.05;
    SRL_FIELD(int, numOfPrevValues) = 5;
    SRL_FIELD(double, quadDecimate) = 1;
    SRL_FIELD(double, searchWindow) = 0.25;
    SRL_FIELD(bool, usePredictive) = true;
    SRL_FIELD(int, calibrationTracker) = 0;
    SRL_FIELD(bool, ignoreTracker0) = false;
    SRL_FIELD(bool, rotateCl) = false;
    SRL_FIELD(bool, rotateCounterCl) = false;
    SRL_FIELD(bool, coloredMarkers) = true;
    SRL_FIELD(double, calibOffsetX) = 0;
    SRL_FIELD(double, calibOffsetY) = 100;
    SRL_FIELD(double, calibOffsetZ) = 100;
    SRL_FIELD(double, calibOffsetA) = 100;
    SRL_FIELD(double, calibOffsetB) = 0;
    SRL_FIELD(double, calibOffsetC) = 0;
    SRL_FIELD(bool, circularWindow) = true;
    SRL_FIELD(double, smoothingFactor) = 0.5;
    SRL_FIELD(int, camFps) = 30;
    SRL_FIELD(int, camHeight) = 0;
    SRL_FIELD(int, camWidth) = 0;
    SRL_FIELD(bool, cameraSettings) = false;
    SRL_FIELD(double camLatency) = 0;
    SRL_FIELD(bool, circularMarkers) = false;
    SRL_FIELD_V(double, trackerCalibDistance,
        { if (value < 0.5) value = 0.5; }) = 0.5;
    SRL_FIELD_V(int, cameraCalibSamples,
        { if (value < 15) value = 15; }) = 15;
    SRL_FIELD(bool, settingsParameters) = false;
    SRL_FIELD(double, cameraAutoexposure) = 0;
    SRL_FIELD(double, cameraExposure) = 0;
    SRL_FIELD(double, cameraGain) = 0;
    SRL_FIELD(bool, trackCalibCenters) = false;
    SRL_FIELD(float, depthSmoothing) = 0;
    SRL_FIELD(float, additionalSmoothing) = 0;
    SRL_FIELD(int, markerLibrary) = 0;
    SRL_FIELD_V(int, markersPerTracker,
        { if (value <= 0) value = 45; }) = 45;
    SRL_FIELD(int, languageSelection) = 0;
    SRL_FIELD_V(double, calibScale,
        { if (value < 0.5) value = 1.0; }) = 1.0;

    // std::string version = "0.5.5";
    // std::string driverversion = "0.5.5";
);

// Non-user editable calibration data, long lists of numbers.
// Potentially store this in a file.yaml.gz to reduce size,
//  and show that it is not user editable. FileStorage has this ability built in
SRL_CLASS(CalibrationStorage, FileStorageSerializer,
    CalibrationStorage()
        : FileStorageSerializer("calibration.yaml")
    {
        Load();
    }

    SRL_FIELD(cv::Mat, camMat);
    SRL_FIELD(cv::Mat, distCoeffs);
    SRL_FIELD(cv::Mat, stdDeviationsIntrinsics);
    SRL_FIELD(std::vector<double>, perViewErrors);
    SRL_FIELD(std::vector<std::vector<cv::Point2f>>, allCharucoCorners);
    SRL_FIELD(std::vector<std::vector<int>>, allCharucoIds);
    SRL_FIELD(std::vector<cv::Ptr<cv::aruco::Board>>, trackers);
    SRL_FIELD(cv::Mat, wtranslation);
    SRL_FIELD(Quaternion<double>, wrotation);

);

SRL_CLASS(ArucoConfigStorage, FileStorageFlatSerializer,
    ArucoConfigStorage()
        : FileStorageFlatSerializer("aruco.yaml")
    {
        auto p = cv::aruco::DetectorParameters::create();
        p->detectInvertedMarker = true;
        p->cornerRefinementMethod = cv::aruco::CORNER_REFINE_CONTOUR;
        params(p);

        Load();
    }

    SRL_FIELD(cv::Ptr<cv::aruco::DetectorParameters>, params)
);
