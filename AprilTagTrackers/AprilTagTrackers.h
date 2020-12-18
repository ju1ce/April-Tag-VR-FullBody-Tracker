// AprilTagTrackers.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include "GUI.h"
#include "Parameters.h"
#include "Connection.h"
#include "Helpers.h"
#include <wx/wx.h>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <thread>

#include <apriltag.h>
#include <tagStandard41h12.h>

class Tracker
{
public:
    Tracker(Parameters*, Connection*);
    void StartCamera(std::string);
    void StartCameraCalib(); 
    void StartTrackerCalib(); 
    void Start();

    bool mainThreadRunning = false;
    bool cameraRunning = false;
    bool previewCamera = false;
    bool recalibrate = false;
    bool manualRecalibrate = false;

    GUI* gui;

    cv::Mat wtranslation = (cv::Mat_<double>(4, 4) << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    Quaternion<double> wrotation = Quaternion<double>(1, 0, 0, 0);

private:
    void CameraLoop();
    void CalibrateCamera();
    void CalibrateTracker();
    void MainLoop();

    int drawImgSize = 480;

    cv::VideoCapture cap;

    cv::Mat retImage;
    bool imageReady = false;   

    Parameters* parameters;
    Connection* connection;

    std::thread cameraThread;
    std::thread mainThread;

    std::vector<cv::Ptr<cv::aruco::Board>> trackers;
    bool trackersCalibrated = false;

    //Quaternion

    //Quaternion<double> q;

    image_u8_t* im;
    void detectMarkersApriltag(cv::Mat, std::vector<std::vector<cv::Point2f> >*, std::vector<int>*, std::vector<cv::Point2f>*, apriltag_detector_t*);  
};

class MyApp : public wxApp
{
    Tracker* tracker;
    Parameters* params;
    Connection* conn;
    GUI* gui;

public:
    virtual int OnExit();
    virtual bool OnInit();
    void ButtonPressedCamera(wxCommandEvent&);
    void ButtonPressedCameraCalib(wxCommandEvent&);
    void ButtonPressedCameraPreview(wxCommandEvent&);
    void ButtonPressedConnect(wxCommandEvent&);
    void ButtonPressedTrackerCalib(wxCommandEvent&);
    void ButtonPressedStart(wxCommandEvent&);
    void ButtonPressedSpaceCalib(wxCommandEvent&);
};
