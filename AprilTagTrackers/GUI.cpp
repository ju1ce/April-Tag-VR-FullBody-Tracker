#include "GUI.h"

GUI::GUI(const wxString& title, Parameters * params)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(350, 650))
{
    wxNotebook* nb = new wxNotebook(this, -1, wxPoint(-1, -1),
        wxSize(-1, -1), wxNB_TOP);

    CameraPage* panel = new CameraPage(nb);
    ParamsPage* panel2 = new ParamsPage(nb, params);

    nb->AddPage(panel, "Camera");
    nb->AddPage(panel2, "Params");

    Centre();
}

CameraPage::CameraPage(wxNotebook* parent)
    :wxPanel(parent)
{
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    wxFlexGridSizer* fgs = new wxFlexGridSizer(5, 2, 20, 20);

    wxButton* btn1 = new wxButton(this, GUI::CAMERA_BUTTON, "1. Start/Stop camera");
    wxButton* btn2 = new wxButton(this, GUI::CAMERA_CALIB_BUTTON, "2. Calibrate camera");
    wxButton* btn4 = new wxButton(this, GUI::CONNECT_BUTTON, "4. Connect to SteamVR");
    wxButton* btn3 = new wxButton(this, GUI::TRACKER_CALIB_BUTTON, "3. Calibrate trackers");
    wxButton* btn5 = new wxButton(this, GUI::START_BUTTON, "5. Start/Stop");

    wxCheckBox* cb = new wxCheckBox(this, GUI::CAMERA_CHECKBOX, wxT("Preview camera"),
        wxPoint(20, 20));
    wxCheckBox* cb2 = new wxCheckBox(this, GUI::SPACE_CALIB_CHECKBOX, wxT("Calibrate playspace"),
        wxPoint(20, 20));
    cb2->SetValue(true);

    fgs->Add(btn1);
    fgs->Add(cb);
    fgs->Add(btn2);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn3);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn4);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn5);
    fgs->Add(cb2);

    hbox->Add(fgs, 1, wxALL | wxEXPAND, 15);
    this->SetSizer(hbox);
}

ParamsPage::ParamsPage(wxNotebook* parent, Parameters* params)
    :wxPanel(parent)
{
    parameters = params;
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    wxFlexGridSizer* fgs = new wxFlexGridSizer(16, 2, 10, 10);

    wxStaticText* cameraAddrText = new wxStaticText(this, -1, wxT("Ip or ID of camera"));
    wxStaticText* trackerNumText = new wxStaticText(this, -1, wxT("Number of trackers"));
    wxStaticText* markerSizeText = new wxStaticText(this, -1, wxT("Size of markers in cm"));
    wxStaticText* prevValuesText = new wxStaticText(this, -1, wxT("Number of values for smoothing"));
    wxStaticText* smoothingText = new wxStaticText(this, -1, wxT("Additional smoothing"));
    wxStaticText* quadDecimateText = new wxStaticText(this, -1, wxT("Quad decimate"));
    wxStaticText* searchWindowText = new wxStaticText(this, -1, wxT("Search window"));
    wxStaticText* usePredictiveText = new wxStaticText(this, -1, wxT("Use previous position as guess"));
    wxStaticText* calibrationTrackerText = new wxStaticText(this, -1, wxT("Tracker to use for calibration"));
    wxStaticText* rotateText = new wxStaticText(this, -1, wxT("Rotate camera 90°"));
    wxStaticText* offsetxText = new wxStaticText(this, -1, wxT("X axis calibration offset"));
    wxStaticText* offsetyText = new wxStaticText(this, -1, wxT("Y axis calibration offset"));
    wxStaticText* offsetzText = new wxStaticText(this, -1, wxT("Z axis calibration offset"));
    wxStaticText* circularText = new wxStaticText(this, -1, wxT("Use circular search window"));
    wxStaticText* camFpsText = new wxStaticText(this, -1, wxT("Camera FPS"));
    wxStaticText* camHeightText = new wxStaticText(this, -1, wxT("Camera height in pixels"));
    wxStaticText* camWitdthText = new wxStaticText(this, -1, wxT("Camera width in pixels"));


    cameraAddrField = new wxTextCtrl(this, -1, parameters->cameraAddr);
    trackerNumField = new wxTextCtrl(this, -1, std::to_string(parameters->trackerNum));
    markerSizeField = new wxTextCtrl(this, -1, std::to_string(parameters->markerSize*100));
    prevValuesField = new wxTextCtrl(this, -1, std::to_string(parameters->numOfPrevValues));
    smoothingField = new wxTextCtrl(this, -1, std::to_string(parameters->smoothingFactor));
    quadDecimateField = new wxTextCtrl(this, -1, std::to_string(parameters->quadDecimate));
    searchWindowField = new wxTextCtrl(this, -1, std::to_string(parameters->searchWindow));
    usePredictiveField = new wxCheckBox(this, -1, wxT(""));
    usePredictiveField->SetValue(parameters->usePredictive);
    calibrationTrackerField = new wxTextCtrl(this, -1, std::to_string(parameters->calibrationTracker));
    rotateField = new wxCheckBox(this, -1, wxT(""));
    rotateField->SetValue(parameters->rotate);
    offsetxField = new wxTextCtrl(this, -1, std::to_string(parameters->calibOffsetX));
    offsetyField = new wxTextCtrl(this, -1, std::to_string(parameters->calibOffsetY));
    offsetzField = new wxTextCtrl(this, -1, std::to_string(parameters->calibOffsetZ));
    circularField = new wxCheckBox(this, -1, wxT(""));
    circularField->SetValue(parameters->circularWindow);
    camFpsField = new wxTextCtrl(this, -1, std::to_string(parameters->camFps));
    camWidthField = new wxTextCtrl(this, -1, std::to_string(parameters->camWidth));
    camHeightField = new wxTextCtrl(this, -1, std::to_string(parameters->camHeight));

    wxButton* btn1 = new wxButton(this, SAVE_BUTTON, "Save");
    Connect(SAVE_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ParamsPage::SaveParams));

    fgs->Add(cameraAddrText);
    fgs->Add(cameraAddrField);
    fgs->Add(trackerNumText);
    fgs->Add(trackerNumField);
    fgs->Add(markerSizeText);
    fgs->Add(markerSizeField);
    fgs->Add(rotateText);
    fgs->Add(rotateField);
    fgs->Add(prevValuesText);
    fgs->Add(prevValuesField);
    fgs->Add(smoothingText);
    fgs->Add(smoothingField);
    fgs->Add(quadDecimateText);
    fgs->Add(quadDecimateField);
    fgs->Add(searchWindowText);
    fgs->Add(searchWindowField);
    fgs->Add(calibrationTrackerText);
    fgs->Add(calibrationTrackerField);
    fgs->Add(usePredictiveText);
    fgs->Add(usePredictiveField);
    fgs->Add(offsetxText);
    fgs->Add(offsetxField);
    fgs->Add(offsetyText);
    fgs->Add(offsetyField);
    fgs->Add(offsetzText);
    fgs->Add(offsetzField);
    fgs->Add(circularText);
    fgs->Add(circularField);
    fgs->Add(camFpsText);
    fgs->Add(camFpsField);
    fgs->Add(camWitdthText);
    fgs->Add(camWidthField);
    fgs->Add(camHeightText);
    fgs->Add(camHeightField);

    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn1);

    hbox->Add(fgs, 1, wxALL | wxEXPAND, 15);
    this->SetSizer(hbox);
}
void ParamsPage::SaveParams(wxCommandEvent& event)
{
    try {
        parameters->cameraAddr = cameraAddrField->GetValue().ToStdString();
        parameters->trackerNum = std::stoi(trackerNumField->GetValue().ToStdString());
        parameters->markerSize = std::stod(markerSizeField->GetValue().ToStdString()) / 100;
        parameters->numOfPrevValues = std::stoi(prevValuesField->GetValue().ToStdString());
        parameters->quadDecimate = std::stod(quadDecimateField->GetValue().ToStdString());
        parameters->searchWindow = std::stod(searchWindowField->GetValue().ToStdString());
        parameters->usePredictive = usePredictiveField->GetValue();
        parameters->calibrationTracker = std::stoi(calibrationTrackerField->GetValue().ToStdString());
        parameters->rotate = rotateField->GetValue();
        parameters->calibOffsetX = std::stod(offsetxField->GetValue().ToStdString());
        parameters->calibOffsetY = std::stod(offsetyField->GetValue().ToStdString());
        parameters->calibOffsetZ = std::stod(offsetzField->GetValue().ToStdString());
        parameters->circularWindow = circularField->GetValue();
        parameters->smoothingFactor = std::stod(smoothingField->GetValue().ToStdString());
        parameters->camFps = std::stoi(camFpsField->GetValue().ToStdString());
        parameters->camWidth = std::stoi(camWidthField->GetValue().ToStdString());
        parameters->camHeight = std::stoi(camHeightField->GetValue().ToStdString());
        parameters->Save();
    }
    catch (std::exception & e)
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Please enter appropriate values."), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
    }
}


