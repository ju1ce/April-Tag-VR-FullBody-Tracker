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

class LicensePage : public wxPanel
{
public:
    LicensePage(wxNotebook* parent);
    std::string ATT_LICENSE = (

        "MIT License \n\n"

        "Copyright(c) 2021 https://github.com/ju1ce/ \n\n"

        "Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand associated documentation files(the \"Software\"),"
        " to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,"
        "and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions : \n\n"

        "The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software. \n\n"

        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES"
        "OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE"
        "FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION"
        "WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE."
        );

    std::string APRILTAG_LICENSE = (

        "BSD 2 - Clause License\n\n"

        "Copyright(C) 2013 - 2016, The Regents of The University of Michigan.All rights reserved.\n\n"

        "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met :\n\n"

        "Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.\n\n"

        "Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer"
        "in the documentation and /or other materials provided with the distribution.\n\n"

        "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES,"
        "INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE"
        "DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,"
        "EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,"
        "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT "
        "LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF"
        "ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

        );

    std::string OPENCV_LICENSE = (
        "Copyright(c) 2021, OpenCV team \n\n"

        "Licensed under the Apache License, Version 2.0 (the \"License\"); "
        "you may not use this file except in compliance with the License. "
        "You may obtain a copy of the License at \n\n"

        "http ://www.apache.org/licenses/LICENSE-2.0 \n\n"

        "Unless required by applicable law or agreed to in writing, software "
        "distributed under the License is distributed on an \"AS IS\" BASIS, "
        "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. "
        "See the License for the specific language governing permissionsand "
        "limitations under the License."
        );

};

class CameraPage : public wxPanel
{
public:
    CameraPage(wxNotebook* parent, GUI* parentGUI, Parameters* params);
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

