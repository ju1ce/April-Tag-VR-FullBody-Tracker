#pragma once

#include "Config.h"
#include "Localization.h"

#pragma warning(push)
#pragma warning(disable : 4996)
#include <wx/wx.h>
#pragma warning(pop)

class Connection;
class GUI;
class Parameters;
class Tracker;

class MyApp : public wxApp
{
    Tracker* tracker;
    Connection* conn;
    GUI* gui;

    // Various configs exist for duration of app
    // Other classes should only use these as (const) references.
    UserConfig user_config;
    CalibrationConfig calib_config;
    ArucoConfig aruco_config;
    Localization lc;

public:
    int OnExit() override;
    bool OnInit() override;

#ifdef ATT_OVERRIDE_ERROR_HANDLERS
    void OnFatalException() override;
    void OnUnhandledException() override;
    bool OnExceptionInMainLoop() override;
#endif

    void ButtonPressedCamera(wxCommandEvent&);
    void ButtonPressedCameraPreview(wxCommandEvent&);
    void ButtonPressedCameraCalib(wxCommandEvent&);
    void ButtonPressedCameraCalibPreview(wxCommandEvent&);
    void ButtonPressedTimeProfile(wxCommandEvent&);
    void ButtonPressedConnect(wxCommandEvent&);
    void ButtonPressedTrackerCalib(wxCommandEvent&);
    void ButtonPressedStart(wxCommandEvent&);
    void ButtonPressedSpaceCalib(wxCommandEvent&);
    void ButtonPressedMulticamAutocalib(wxCommandEvent&);
    void ButtonPressedLockHeight(wxCommandEvent&);
    void ButtonPressedDisableOut(wxCommandEvent&);
    void ButtonPressedDisableOpenVrApi(wxCommandEvent&);
};
