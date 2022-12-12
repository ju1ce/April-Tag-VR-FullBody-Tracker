#pragma once

#include "Config.hpp"
#include "GUI.hpp"
#include "Quaternion.hpp"
#include "RefPtr.hpp"

#include <opencv2/core/affine.hpp>
#include <opencv2/videoio.hpp>

#include <chrono>
#include <mutex>
#include <thread>

struct TrackerStatus
{
    cv::Vec3d boardRvec, boardTvec, boardTvecDriver;
    bool boardFound, boardFoundDriver;
    std::vector<std::vector<double>> prevLocValues;
    cv::Point2d maskCenter;
    std::chrono::milliseconds last_update_timestamp;
    int searchSize;
};

struct FrameData
{
    bool ready = false;
    cv::Mat image;
    std::chrono::steady_clock::time_point time;
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
    void CopyFreshCameraImageTo(FrameData& frame);
    void CalibrateCamera();
    void CalibrateCameraCharuco();
    void CalibrateTracker();
    void MainLoop();

    void HandleConnectionErrors();

    /// Sets the wtransform, wrotation, and wscale
    void SetWorldTransform(const cfg::ManualCalib::Real& calib);
    /// Calibration transformation
    cv::Affine3d wtransform;
    /// wtransform rotation part as a quaternion
    cv::Quatd wrotation;
    double wscale = 1;

    int drawImgSize = 1385;

    cv::VideoCapture cap;

    // cameraFrame is protected by cameraImageMutex.
    // Use CopyFreshCameraImageTo in order to get the latest camera image.
    std::mutex cameraImageMutex;
    FrameData cameraFrame;

    std::unique_ptr<Connection> connection;

    UserConfig& user_config;
    CalibrationConfig& calib_config;
    const ArucoConfig& aruco_config;
    const Localization& lc;

    std::thread cameraThread;
    std::thread mainThread;

    std::vector<cv::Ptr<cv::aruco::Board>> trackers;
    bool trackersCalibrated = false;

    std::chrono::steady_clock::time_point last_frame_time;
};
