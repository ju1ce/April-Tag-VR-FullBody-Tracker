#pragma once

#pragma warning(push)
#pragma warning(disable:4996)
#include <wx/wx.h>
#pragma warning(pop)

class Connection;
class GUI;
class Parameters;
class Tracker;

class MyApp : public wxApp
{
    Tracker* tracker;
    Parameters* params;
    Connection* conn;
    GUI* gui;

public:
    virtual int OnExit() wxOVERRIDE;
    virtual bool OnInit() wxOVERRIDE;
    void ButtonPressedCamera(wxCommandEvent&);
    void ButtonPressedCameraPreview(wxCommandEvent&);
    void ButtonPressedCameraCalib(wxCommandEvent&);
    void ButtonPressedCameraCalibPreview(wxCommandEvent&);
    void ButtonPressedConnect(wxCommandEvent&);
    void ButtonPressedTrackerCalib(wxCommandEvent&);
    void ButtonPressedStart(wxCommandEvent&);
    void ButtonPressedSpaceCalib(wxCommandEvent&);
    void ButtonPressedMulticamAutocalib(wxCommandEvent&);
    void ButtonPressedLockHeight(wxCommandEvent&);
};
