// AprilTagTrackers.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <memory>

#include <wx/wx.h>

class Connection;
class GUI;
class Parameters;
class Tracker;

class MyApp : public wxApp
{
    std::shared_ptr<Tracker> tracker;
    std::shared_ptr<Parameters> params;
    std::shared_ptr<Connection> conn;
    std::shared_ptr<GUI> gui;

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
