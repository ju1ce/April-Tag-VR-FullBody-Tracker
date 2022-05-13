#pragma once

#include "MyApp.h"

#include "Quaternion.h"
#include "Config.h"
#include "Localization.h"
#include <opencv2/videoio.hpp>
#include <thread>
#include <mutex>
#include <chrono>

struct TrackerStatus {
    cv::Vec3d boardRvec, boardTvec, boardTvecDriver;
    bool boardFound, boardFoundDriver;
    std::vector<std::vector<double>> prevLocValues;
    cv::Point2d maskCenter;
};

class Connection;
class GUI;
class Parameters;

class Tracker
{
public:
    Tracker(MyApp* myApp, Connection* connection, UserConfig& user_config, CalibrationConfig& calib_config, const Localization& lcl, const ArucoConfig& aruco_config);
    void StartCamera(std::string id, int apiPreference);
    void StartCameraCalib();
    void StartTrackerCalib();
    void Start();

    bool mainThreadRunning = false;
    bool cameraRunning = false;
    bool showCameraPreview = false;
    bool showOutPreview = true;
    bool previewCameraCalibration = false;
    bool showTimeProfile = false;
    bool recalibrate = false;
    bool manualRecalibrate = false;
    bool multicamAutocalib = false;
    bool lockHeightCalib = false;
    int messageDialogResponse = wxID_CANCEL;

    GUI* gui;

    cv::Mat wtranslation = (cv::Mat_<double>(4, 4) << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    Quaternion<double> wrotation = Quaternion<double>(1, 0, 0, 0);

    double calibScale = 1;

private:
    void CameraLoop();
    void CopyFreshCameraImageTo(cv::Mat& image);
    void CalibrateCamera();
    void CalibrateCameraCharuco();
    void CalibrateTracker();
    void MainLoop();

    int drawImgSize = 480;

    cv::VideoCapture cap;

    // cameraImage and imageReady are protected by cameraImageMutex.
    // Use CopyFreshCameraImageTo in order to get the latest camera image.
    std::mutex cameraImageMutex;
    cv::Mat cameraImage;
    bool imageReady = false;

    UserConfig& user_config;
    CalibrationConfig& calib_config;
    const Localization& lc;
    const ArucoConfig& aruco_config;

    Connection* connection;

    std::thread cameraThread;
    std::thread mainThread;

    std::vector<cv::Ptr<cv::aruco::Board>> trackers;
    bool trackersCalibrated = false;

    //Quaternion

    //Quaternion<double> q;

    std::chrono::steady_clock::time_point last_frame_time;

    MyApp* parentApp;
};
