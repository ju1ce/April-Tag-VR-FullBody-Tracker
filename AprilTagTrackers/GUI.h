#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include "Parameters.h"

class ValueInput : public wxPanel
{
public:
    ValueInput(wxPanel* parent, std::string nm, double val);
    double value;
    void SetValue(double val);

private:
    wxButton* dwn1;
    wxButton* dwn2;
    wxTextCtrl* input = 0;
    wxButton* up1;
    wxButton* up2;

    void ButtonPressed(wxCommandEvent&);
    void MouseScroll(wxMouseEvent&);
};

class GUI : public wxFrame
{
public:
    GUI(const wxString& title, std::shared_ptr<Parameters> params);
    static const int CAMERA_BUTTON = 1;
    static const int CAMERA_CALIB_BUTTON = 3;
    static const int CAMERA_CHECKBOX = 4;
    static const int CONNECT_BUTTON = 5;
    static const int TRACKER_CALIB_BUTTON = 6;
    static const int START_BUTTON = 7;
    static const int SPACE_CALIB_CHECKBOX = 8;
    static const int MANUAL_CALIB_CHECKBOX = 9;

    ValueInput *manualCalibX;
    ValueInput *manualCalibY;
    ValueInput *manualCalibZ;
    ValueInput *manualCalibA;
    ValueInput *manualCalibB;
    ValueInput *manualCalibC;

    wxBoxSizer* posHbox;
    wxBoxSizer* rotHbox;

    wxCheckBox* cb2;
    wxCheckBox* cb3;
};

class CameraPage : public wxPanel
{
public:
    CameraPage(wxNotebook* parent, GUI* parentGUI);
};

class ParamsPage : public wxPanel
{
public:
    ParamsPage(wxNotebook* parent, std::shared_ptr<Parameters> params);

private:
    const int SAVE_BUTTON = 2;
    const int HELP_BUTTON = 10;
    std::shared_ptr<Parameters> parameters;
    wxTextCtrl* cameraAddrField;
    wxTextCtrl* camFpsField;
    wxTextCtrl* camWidthField;
    wxTextCtrl* camHeightField;
    wxTextCtrl* camLatencyField;
    wxTextCtrl* trackerNumField;
    wxTextCtrl* markerSizeField;
    wxTextCtrl* prevValuesField;
    wxTextCtrl* quadDecimateField;
    wxTextCtrl* searchWindowField;
    wxCheckBox* usePredictiveField;
    wxCheckBox* ignoreTracker0Field;
    wxTextCtrl* calibrationTrackerField;
    wxCheckBox* rotateClField;
    wxCheckBox* rotateCounterClField;
    wxTextCtrl* offsetxField;
    wxTextCtrl* offsetyField;
    wxTextCtrl* offsetzField;
    wxCheckBox* circularField;
    wxTextCtrl* smoothingField;
    wxCheckBox* cameraSettingsField;
    wxCheckBox* chessboardCalibField;
    void SaveParams(wxCommandEvent&);
    void ShowHelp(wxCommandEvent&);
};
