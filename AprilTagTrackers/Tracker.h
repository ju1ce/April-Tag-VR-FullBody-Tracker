// AprilTagTrackers.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "Quaternion.h"

class Connection;
class GUI;
class Parameters;

class Tracker
{
public:
    Tracker(std::shared_ptr<Parameters>, std::shared_ptr<Connection>);
    void StartCamera(std::string);
    void StartCameraCalib();
    void StartTrackerCalib();
    void Start();

    bool mainThreadRunning = false;
    bool cameraRunning = false;
    bool previewCamera = false;
    bool recalibrate = false;
    bool manualRecalibrate = false;

    std::shared_ptr<GUI> gui;

    cv::Mat wtranslation = (cv::Mat_<double>(4, 4) << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    Quaternion<double> wrotation = Quaternion<double>(1, 0, 0, 0);

private:
    void CameraLoop();
    void CalibrateCamera();
    void CalibrateCameraCharuco();
    void CalibrateTracker();
    void MainLoop();

    int drawImgSize = 480;

    cv::VideoCapture cap;

    cv::Mat retImage;
    bool imageReady = false;

    std::shared_ptr<Parameters> parameters;
    std::shared_ptr<Connection> connection;

    std::thread cameraThread;
    std::thread mainThread;

    std::vector<cv::Ptr<cv::aruco::Board>> trackers;
    bool trackersCalibrated = false;

    //Quaternion

    //Quaternion<double> q;

    clock_t last_frame_time;
};
