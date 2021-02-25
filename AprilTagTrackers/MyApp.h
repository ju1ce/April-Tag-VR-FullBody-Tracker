// AprilTagTrackers.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <wx/wx.h>

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
    void ButtonPressedCameraCalib(wxCommandEvent&);
    void ButtonPressedCameraPreview(wxCommandEvent&);
    void ButtonPressedConnect(wxCommandEvent&);
    void ButtonPressedTrackerCalib(wxCommandEvent&);
    void ButtonPressedStart(wxCommandEvent&);
    void ButtonPressedSpaceCalib(wxCommandEvent&);
};
