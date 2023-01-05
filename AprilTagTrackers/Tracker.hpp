#pragma once

#include "Config.hpp"
#include "GUI.hpp"
#include "RefPtr.hpp"
#include "tracker/OpenVRClient.hpp"
#include "tracker/TrackerUnit.hpp"
#include "tracker/VideoCapture.hpp"
#include "tracker/VRDriver.hpp"
#include "utils/Error.hpp"
#include "utils/SteadyTimer.hpp"
#include "utils/StrongType.hpp"
#include "utils/Types.hpp"

#include <opencv2/core/affine.hpp>
#include <opencv2/videoio.hpp>

#include <condition_variable>
#include <mutex>
#include <optional>
#include <ranges>
#include <thread>

class PlayspaceCalib
{
public:
    void Set(const cv::Vec3d& posOffset, const cv::Vec3d& angleOffset, double scale)
    {
        cv::Matx33d rotMat = EulerAnglesToRotationMatrix(angleOffset);
        mTransform = cv::Affine3d(rotMat, posOffset);
        mRotation = cv::Quatd::createFromRotMat(rotMat).normalize();
        mInvTransform = mTransform.inv();
        mInvRotation = mRotation.inv();
        mScale = scale;
    }
    void Set(cfg::ManualCalib::Real calib)
    {
        Set(calib.posOffset, calib.angleOffset, calib.scale);
    }

    Pose Transform(const Pose& pose) const
    {
    Pose InvTransform(const Pose& pose) const
    {
        return {mInvTransform * pose.position, mInvRotation * pose.rotation};
    }
    cv::Point3d Transform(const cv::Point3d& pos) const { return mTransform * pos; }

    double GetScale() const { return mScale; }

private:
    cv::Affine3d mTransform{};
    cv::Quatd mRotation{};
    cv::Affine3d mInvTransform{};
    cv::Quatd mInvRotation{};
    double mScale = 0;
};

struct TrackerStatus
{
    cv::Vec3d boardRvec, boardTvec, boardTvecDriver;
    bool boardFound, boardFoundDriver;
    cv::Point2d maskCenter;
};

struct CapturedFrame
{
    cv::Mat image;
    utils::SteadyTimer::TimePoint timestamp;
};

class MainLoopRunner;
class PlayspaceCalibrator;

class Tracker : public ITrackerControl
{
    static constexpr int DRAW_IMG_SIZE = 480;

    static inline const cv::Scalar COLOR_MARKER_DETECTED{0, 0, 255}; /// blue
    static inline const cv::Scalar COLOR_MARKER_ADDING{255, 0, 255}; /// yellow
    static inline const cv::Scalar COLOR_MARKER_ADDED{0, 255, 0}; /// green
    static inline const cv::Scalar COLOR_MARKER_FAR{255, 0, 255}; /// purple
    static inline const cv::Scalar COLOR_MASK{255, 0, 0}; /// red

public:
    friend class MainLoopRunner;
    friend class PlayspaceCalibrator;

    Tracker(const Tracker&) = delete;
    Tracker(Tracker&&) = delete;
    /// config and locale references are expected to exceed lifetime of this instance
    Tracker(UserConfig& _userConfig, CalibrationConfig& _calibConfig, ArucoConfig& _arucoConfig, const Localization& _lc);
    void StartCamera(RefPtr<cfg::Camera> cam);
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
    void CopyFreshCameraImageTo(CapturedFrame& frame);
    void CalibrateCamera();
    void CalibrateCameraCharuco();
    void CalibrateTracker();
    void MainLoop();

    void SetTrackerUnitsFromConfig();
    void SaveTrackerUnitsToCalib(const std::vector<tracker::TrackerUnit>&);
    bool IsTrackerUnitsCalibrated() const
    {
        return std::all_of(mTrackerUnits.begin(), mTrackerUnits.end(),
                           [](const auto& unit) { return unit.IsCalibrated(); });
    }

    void StartCameraThread()
    {
        ATT_ASSERT(utils::IsMainThread());
        StopWorkThread();
        cameraRunning = true;
        cameraThread = std::thread(&Tracker::CameraLoop, this);
    }
    void StopCameraThread()
    {
        ATT_ASSERT(utils::IsMainThread());
        StopWorkThread();
        cameraRunning = false;
        if (cameraThread.joinable()) cameraThread.join();
    }
    void StartWorkThread()
    {
    }
    void StopWorkThread()
    {
    }
    void WorkThreadError(const U8String& msg)
    {
        mainThreadRunning = false;
        gui->ShowPopup(msg, PopupStyle::Error);
    }

    /// Sets the wtransform, wrotation, and wscale
    // TODO: rename uses of world transform and manual calib to playspace calibration
    void SetWorldTransform(const cfg::ManualCalib::Real& calib);
    /// Calibration transformation
    cv::Affine3d wtransform;
    /// wtransform rotation part as a quaternion
    cv::Quatd wrotation;
    double wscale = 1;

    tracker::VideoCapture mCapture;

    // mImageReadyCond, mIsImageReady, and mCameraFrame are protected by mCameraImageMutex.
    // Use CopyFreshCameraImageTo in order to get the latest camera image.
    std::mutex mCameraImageMutex{};
    std::condition_variable mImageReadyCond{};
    bool mIsImageReady = false;
    CapturedFrame mCameraFrame{};

    UserConfig& user_config;
    CalibrationConfig& calib_config;
    const ArucoConfig& aruco_config;
    const Localization& lc;

    std::thread cameraThread;
    std::thread mainThread;

    utils::SteadyTimer mFrameTimer{};

    std::unique_ptr<tracker::IVRClient> mVRClient{};
    std::optional<tracker::VRDriver> mVRDriver{};

    std::vector<tracker::TrackerUnit> mTrackerUnits;
};
