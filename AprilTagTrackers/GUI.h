#pragma once
#pragma warning(push)
#pragma warning(disable:4996)
#include <wx/wx.h>
#include <wx/notebook.h>
#pragma warning(pop)

#include "Parameters.h"
#include "Connection.h"

class ValueInput : public wxPanel
{
public:
    ValueInput(wxPanel* parent, const wxString& nm, double val);
    double value;
    void SetValue(double val);

private:
    wxTextCtrl* input = 0;

    void ButtonPressed(wxCommandEvent&);
    void MouseScroll(wxMouseEvent&);
};

class GUI : public wxFrame
{
public:
    GUI(const wxString& title, Parameters* params, Connection* conn);
    static const int CAMERA_BUTTON = 1;
    static const int CAMERA_CHECKBOX = 2;
    static const int CAMERA_CALIB_BUTTON = 3;
    static const int CAMERA_CALIB_CHECKBOX = 4;

    static const int CONNECT_BUTTON = 5;
    static const int TRACKER_CALIB_BUTTON = 6;
    static const int START_BUTTON = 7;
    static const int SPACE_CALIB_CHECKBOX = 8;
    static const int MANUAL_CALIB_CHECKBOX = 9;
    static const int MULTICAM_AUTOCALIB_CHECKBOX = 10;
    static const int LOCK_HEIGHT_CHECKBOX = 11;

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
    wxCheckBox* cb4;
    wxCheckBox* cb5;
};

class CameraPage : public wxPanel
{
public:
    CameraPage(wxNotebook* parent, GUI* parentGUI);
};

class ParamsPage : public wxPanel
{
public:
    ParamsPage(wxNotebook* parent, Parameters* params, Connection* conn);

private:
    const int SAVE_BUTTON = 2;
    const int HELP_BUTTON = 10;
    Parameters* parameters;
    Connection* connection;
    wxTextCtrl* cameraAddrField;
    wxTextCtrl* cameraApiField;
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
    wxCheckBox* coloredMarkersField;
    wxTextCtrl* offsetxField;
    wxTextCtrl* offsetyField;
    wxTextCtrl* offsetzField;
    wxCheckBox* circularField;
    wxTextCtrl* smoothingField;
    wxCheckBox* cameraSettingsField;
    wxCheckBox* settingsParametersField;
    wxTextCtrl* cameraAutoexposureField;
    wxTextCtrl* cameraExposureField;
    wxTextCtrl* cameraGainField;
    wxCheckBox* chessboardCalibField;
    wxCheckBox* trackerCalibCentersField;
    wxTextCtrl* depthSmoothingField;
    wxTextCtrl* additionalSmoothingField;
    wxChoice* markerLibraryField;
    void SaveParams(wxCommandEvent&);
    void ShowHelp(wxCommandEvent&);

};
