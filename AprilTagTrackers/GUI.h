#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include "Parameters.h"

class GUI : public wxFrame
{
public:
    GUI(const wxString& title, Parameters* params);
    static const int CAMERA_BUTTON = 1;
    static const int CAMERA_CALIB_BUTTON = 3;
    static const int CAMERA_CHECKBOX = 4;
    static const int CONNECT_BUTTON = 5;
    static const int TRACKER_CALIB_BUTTON = 6;
    static const int START_BUTTON = 7;
    static const int SPACE_CALIB_CHECKBOX = 8;
};

class CameraPage : public wxPanel
{
public:
    CameraPage(wxNotebook* parent);
};

class ParamsPage : public wxPanel
{
public:
    ParamsPage(wxNotebook* parent, Parameters* params);

private:
    const int SAVE_BUTTON = 2;
    Parameters* parameters;
    wxTextCtrl* cameraAddrField;
    wxTextCtrl* trackerNumField;
    wxTextCtrl* markerSizeField;
    wxTextCtrl* prevValuesField;
    wxTextCtrl* quadDecimateField;
    wxTextCtrl* searchWindowField;
    wxCheckBox* usePredictiveField;
    wxTextCtrl* calibrationTrackerField;
    wxCheckBox* rotateField;
    wxTextCtrl* offsetxField;
    wxTextCtrl* offsetyField;
    wxTextCtrl* offsetzField;
    wxCheckBox* circularField;
    wxTextCtrl* smoothingField;
    void SaveParams(wxCommandEvent& );
};
