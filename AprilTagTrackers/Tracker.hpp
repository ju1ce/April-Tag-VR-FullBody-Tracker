#pragma once

#include "Config.hpp"
#include "GUI.hpp"
#include "Quaternion.hpp"
#include "RefPtr.hpp"
#include "tracker/VideoCapture.hpp"

#include <opencv2/core/affine.hpp>
#include <opencv2/videoio.hpp>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

struct TrackerStatus
{
    cv::Vec3d boardRvec, boardTvec, boardTvecDriver;
    bool boardFound, boardFoundDriver;
    std::vector<std::vector<double>> prevLocValues;
    cv::Point2d maskCenter;
};

class Connection;

class Tracker : public ITrackerControl
{
public:
    Tracker(const Tracker&) = delete;
    Tracker(Tracker&&) = delete;
    /// config and locale references are expected to exceed lifetime of this instance
    Tracker(UserConfig& _userConfig, CalibrationConfig& _calibConfig, ArucoConfig& _arucoConfig, const Localization& _lc);
    void StartCamera(RefPtr<cfg::CameraInfo> cam);
    void StartCamera() override;
    void StartCameraCalib() override;
    void StartTrackerCalib() override;
    void StartConnection() override;
    void Start() override;
    void Stop() override;
    void UpdateConfig() override;

    bool mainThreadRunning = false;
    bool cameraRunning = false;
    bool showTimeProfile = false;

private:
    void CameraLoop();
    void CopyFreshCameraImageTo(cv::Mat& image);
    void CalibrateCamera();
    void CalibrateCameraCharuco();
    void CalibrateTracker();
    void UpdatePlayspaceCalibrator(bool& posActive, bool& angleActive, cv::Vec3d& posOffset, cv::Vec3d& angleOffset, utils::SteadyTimer& timer);
    void MainLoop();

    void HandleConnectionErrors();

    /// Sets the wtransform, wrotation, and wscale
    void SetWorldTransform(const cfg::ManualCalib::Real& calib);
    /// Calibration transformation
    cv::Affine3d wtransform;
    /// wtransform rotation part as a quaternion
    cv::Quatd wrotation;
    double wscale = 1;

    int drawImgSize = 480;

    tracker::VideoCapture mCapture;

    // cameraImage and imageReady are protected by cameraImageMutex.
    // Use CopyFreshCameraImageTo in order to get the latest camera image.
    std::mutex mCameraImageMutex{};
    cv::Mat mCameraImage{};
    std::condition_variable mImageReadyCond{};
    bool mIsImageReady = false;

    UserConfig& user_config;
    CalibrationConfig& calib_config;
    const ArucoConfig& aruco_config;
    const Localization& lc;

    std::thread cameraThread;
    std::thread mainThread;

    std::vector<cv::Ptr<cv::aruco::Board>> trackers;
    bool trackersCalibrated = false;
    utils::SteadyTimer mFrameTimer{};
};
