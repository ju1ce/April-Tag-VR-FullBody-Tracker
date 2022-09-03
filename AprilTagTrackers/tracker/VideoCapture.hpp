#pragma once

#include "config/VideoStream.hpp"
#include "RefPtr.hpp"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <memory>

namespace tracker
{

class VideoCapture
{
public:
    /// fake api for user to specify custom ps3eye capture implementation
    /// must not conflict with existing cv::VideoCapture api
    static constexpr int CAP_PS3EYE = 9100;

    /// instance is linked to a CameraInfo, so a list of VideoCaptures will sync with the list of cameras in gui
    explicit VideoCapture(RefPtr<cfg::CameraInfo> cameraInfo) : mCameraInfo(cameraInfo) {}

    bool TryOpen();
    void Close();
    /// blocks till frame is ready, matches fps of camera
    bool TryReadFrame(cv::Mat& outImage);
    bool IsOpen() const { return mCapture && mCapture->isOpened(); }

private:
    bool TryOpenCapture();
    bool TryOpenGStreamerCapture(int index);
    void SetCaptureOptions();
    void LogCaptureOptions();

    RefPtr<cfg::CameraInfo> mCameraInfo;
    std::unique_ptr<cv::VideoCapture> mCapture = std::make_unique<cv::VideoCapture>();
};

} // namespace tracker
