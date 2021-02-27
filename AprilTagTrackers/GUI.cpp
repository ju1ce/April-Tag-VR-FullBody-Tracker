#include "GUI.h"

GUI::GUI(const wxString& title, Parameters * params)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(350, 700))
{
    wxNotebook* nb = new wxNotebook(this, -1, wxPoint(-1, -1),
        wxSize(-1, -1), wxNB_TOP);

    CameraPage* panel = new CameraPage(nb,this);
    ParamsPage* panel2 = new ParamsPage(nb, params);

    nb->AddPage(panel, "Camera");
    nb->AddPage(panel2, "Params");

    Centre();
}

CameraPage::CameraPage(wxNotebook* parent,GUI* parentGUI)
    :wxPanel(parent)
{
    wxBoxSizer* hbox = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* fgs = new wxFlexGridSizer(2, 20, 20);

    wxButton* btn1 = new wxButton(this, GUI::CAMERA_BUTTON, "1. Start/Stop camera");
    wxButton* btn2 = new wxButton(this, GUI::CAMERA_CALIB_BUTTON, "2. Calibrate camera");
    wxButton* btn4 = new wxButton(this, GUI::CONNECT_BUTTON, "5. Connect to SteamVR");
    wxButton* btn3 = new wxButton(this, GUI::TRACKER_CALIB_BUTTON, "3. Calibrate trackers");
    wxButton* btn5 = new wxButton(this, GUI::START_BUTTON, "6. Start/Stop");

    wxCheckBox* cb = new wxCheckBox(this, GUI::CAMERA_CHECKBOX, wxT("Preview camera"),
        wxPoint(20, 20));
    //parentGUI->cb2 = new wxCheckBox(this, GUI::SPACE_CALIB_CHECKBOX, wxT("Calibrate playspace"),
    //    wxPoint(20, 20));
    parentGUI->cb3 = new wxCheckBox(this, GUI::MANUAL_CALIB_CHECKBOX, wxT("Calibration mode"),
        wxPoint(20, 20));
    //parentGUI->cb2->SetValue(false);

    fgs->Add(btn1);
    fgs->Add(cb);
    fgs->Add(btn2);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn3);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(new wxStaticText(this, -1, wxT("4. Start up SteamVR!")), 0, wxEXPAND);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn4);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn5);
    //fgs->Add(parentGUI->cb2);
    //fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(parentGUI->cb3);

    hbox->Add(fgs, 1, wxALL | wxEXPAND, 15);

    //hbox->Add(cb3, 1, wxALL | wxEXPAND, 15);

    parentGUI->posHbox = new wxBoxSizer(wxVERTICAL);
    parentGUI->rotHbox = new wxBoxSizer(wxVERTICAL);

    parentGUI->manualCalibX = new ValueInput(this, "X(cm):", 0);
    parentGUI->manualCalibY = new ValueInput(this, "Y(cm):", 0);
    parentGUI->manualCalibZ = new ValueInput(this, "Z(cm):", 0);
    parentGUI->manualCalibA = new ValueInput(this, wxString::FromUTF8("A(°):"), 0);
    parentGUI->manualCalibB = new ValueInput(this, wxString::FromUTF8("B(°):"), 0);
    parentGUI->manualCalibC = new ValueInput(this, wxString::FromUTF8("C(°):"), 0);

    parentGUI->posHbox->Add(new wxStaticText(this, -1, wxT("Disable SteamVR home to see the camera.\nUse the values bellow to align the virtual camera with \nyour IRL one. Use your controllers as references.\nUncheck Calibration mode when done!")), 0, wxEXPAND);
    parentGUI->posHbox->Add(parentGUI->manualCalibX, 1, wxALL | wxEXPAND, 5);
    parentGUI->posHbox->Add(parentGUI->manualCalibY, 1, wxALL | wxEXPAND, 5);
    parentGUI->posHbox->Add(parentGUI->manualCalibZ, 1, wxALL | wxEXPAND, 5);
    parentGUI->rotHbox->Add(parentGUI->manualCalibA, 1, wxALL | wxEXPAND, 5);
    parentGUI->rotHbox->Add(parentGUI->manualCalibB, 1, wxALL | wxEXPAND, 5);
    parentGUI->rotHbox->Add(parentGUI->manualCalibC, 1, wxALL | wxEXPAND, 5);

    hbox->Add(parentGUI->posHbox, 1, wxALL | wxEXPAND, 15);
    hbox->Add(parentGUI->rotHbox, 1, wxALL | wxEXPAND, 15);

    //hbox2->Show(false);

    this->SetSizer(hbox);

    //parentGUI->posHbox->Show(false);
    //parentGUI->rotHbox->Show(false);
}

ParamsPage::ParamsPage(wxNotebook* parent, Parameters* params)
    :wxPanel(parent)
{
    parameters = params;

    cameraAddrField = new wxTextCtrl(this, -1, parameters->cameraAddr);
    trackerNumField = new wxTextCtrl(this, -1, std::to_string(parameters->trackerNum));
    markerSizeField = new wxTextCtrl(this, -1, std::to_string(parameters->markerSize*100));
    prevValuesField = new wxTextCtrl(this, -1, std::to_string(parameters->numOfPrevValues));
    smoothingField = new wxTextCtrl(this, -1, std::to_string(parameters->smoothingFactor));
    quadDecimateField = new wxTextCtrl(this, -1, std::to_string(parameters->quadDecimate));
    searchWindowField = new wxTextCtrl(this, -1, std::to_string(parameters->searchWindow));
    //usePredictiveField = new wxCheckBox(this, -1, wxT(""));
    //usePredictiveField->SetValue(parameters->usePredictive);
    //calibrationTrackerField = new wxTextCtrl(this, -1, std::to_string(parameters->calibrationTracker));
    ignoreTracker0Field = new wxCheckBox(this, -1, wxT(""));
    ignoreTracker0Field->SetValue(parameters->ignoreTracker0);
    rotateClField = new wxCheckBox(this, -1, wxT(""));
    rotateClField->SetValue(parameters->rotateCl);
    rotateCounterClField = new wxCheckBox(this, -1, wxT(""));
    rotateCounterClField->SetValue(parameters->rotateCounterCl);
    //offsetxField = new wxTextCtrl(this, -1, std::to_string(parameters->calibOffsetX));
    //offsetyField = new wxTextCtrl(this, -1, std::to_string(parameters->calibOffsetY));
    //offsetzField = new wxTextCtrl(this, -1, std::to_string(parameters->calibOffsetZ));
    //circularField = new wxCheckBox(this, -1, wxT(""));
    //circularField->SetValue(parameters->circularWindow);
    camFpsField = new wxTextCtrl(this, -1, std::to_string(parameters->camFps));
    camWidthField = new wxTextCtrl(this, -1, std::to_string(parameters->camWidth));
    camHeightField = new wxTextCtrl(this, -1, std::to_string(parameters->camHeight));
    camLatencyField = new wxTextCtrl(this, -1, std::to_string(parameters->camLatency));
    //cameraSettingsField = new wxCheckBox(this, -1, wxT(""));
    //cameraSettingsField->SetValue(parameters->cameraSettings);
    chessboardCalibField = new wxCheckBox(this, -1, wxT(""));
    chessboardCalibField->SetValue(parameters->chessboardCalib);

    wxBoxSizer* hbox = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* fgs = new wxFlexGridSizer(2, 10, 10);

    wxStaticText* cameraAddrText = new wxStaticText(this, -1, wxT("Ip or ID of camera"));
    cameraAddrText->SetToolTip("Will be a number 0-10 for USB cameras and \nhttp://'ip - here':8080/video for IP webcam");
    fgs->Add(cameraAddrText);
    fgs->Add(cameraAddrField);
    wxStaticText* trackerNumText = new wxStaticText(this, -1, wxT("Number of trackers"));
    trackerNumText->SetToolTip("Set to 3 for full body. 2 will not work in vrchat!");
    fgs->Add(trackerNumText);
    fgs->Add(trackerNumField);
    wxStaticText* markerSizeText = new wxStaticText(this, -1, wxT("Size of markers in cm"));
    markerSizeText->SetToolTip("Measure the white square on markers and input it here");
    fgs->Add(markerSizeText);
    fgs->Add(markerSizeField);
    wxStaticText* rotateClText = new wxStaticText(this, -1, wxT("Rotate camera clockwise"));
    rotateClText->SetToolTip(wxString::FromUTF8("Rotate the camera. Use both to rotate image 180°"));
    fgs->Add(rotateClText);
    fgs->Add(rotateClField);
    wxStaticText* rotateCounterClText = new wxStaticText(this, -1, wxT("Rotate camera counterclockwise"));
    rotateCounterClText->SetToolTip(wxString::FromUTF8("Rotate the camera. Use both to rotate image 180°"));
    fgs->Add(rotateCounterClText);
    fgs->Add(rotateCounterClField);
    wxStaticText* prevValuesText = new wxStaticText(this, -1, wxT("Number of values for smoothing"));
    prevValuesText->SetToolTip("Used to remove pose outliers. Can usually be lowered to 3 to reduce latency.");
    fgs->Add(prevValuesText);
    fgs->Add(prevValuesField);
    wxStaticText* smoothingText = new wxStaticText(this, -1, wxT("Additional smoothing"));
    smoothingText->SetToolTip("0 to be fast, but very shaky, 1 to barely move the tracker, but smoothly. Experiment to find the sweet spot");
    fgs->Add(smoothingText);
    fgs->Add(smoothingField);
    wxStaticText* quadDecimateText = new wxStaticText(this, -1, wxT("Quad decimate"));
    quadDecimateText->SetToolTip("Can be 1, 1.5, 2, 3, 4. Higher values will increase FPS, but reduce maximum range of detections");
    fgs->Add(quadDecimateText);
    fgs->Add(quadDecimateField);
    wxStaticText* searchWindowText = new wxStaticText(this, -1, wxT("Search window"));
    searchWindowText->SetToolTip("Size of the search window. Smaller window will speed up detection, but having it too small will cause detection to fail if tracker moves too far in one frame.");
    fgs->Add(searchWindowText);
    fgs->Add(searchWindowField);
    //wxStaticText* calibrationTrackerText = new wxStaticText(this, -1, wxT("Tracker to use for calibration"));
    //fgs->Add(calibrationTrackerText);
    //fgs->Add(calibrationTrackerField);
    wxStaticText* ignoreTracker0Text = new wxStaticText(this, -1, wxT("Ignore tracker 0"));
    ignoreTracker0Text->SetToolTip("If you want to replace the hip tracker with a vive tracker/owotrack, check this option. Keep number of trackers on 3.");
    fgs->Add(ignoreTracker0Text);
    fgs->Add(ignoreTracker0Field);
    //wxStaticText* usePredictiveText = new wxStaticText(this, -1, wxT("Use previous position as guess"));
    //usePredictiveText->SetToolTip("Help tracker detection by using previous pose. There shouldn't be any reason to disable this.");
    //fgs->Add(usePredictiveText);
    //fgs->Add(usePredictiveField);
    //wxStaticText* offsetxText = new wxStaticText(this, -1, wxT("X axis calibration offset"));
    //fgs->Add(offsetxText);
    //fgs->Add(offsetxField);
    //wxStaticText* offsetyText = new wxStaticText(this, -1, wxT("Y axis calibration offset"));
    //fgs->Add(offsetyText);
    //fgs->Add(offsetyField);
    //wxStaticText* offsetzText = new wxStaticText(this, -1, wxT("Z axis calibration offset"));
    //fgs->Add(offsetzText);
    //fgs->Add(offsetzField);
    //wxStaticText* circularText = new wxStaticText(this, -1, wxT("Use circular search window"));
    //circularText->SetToolTip("Use a circle as a search window instead of searching in vertical bands. There should be no reason to disable this.");
    //fgs->Add(circularText);
    //fgs->Add(circularField);
    wxStaticText* camFpsText = new wxStaticText(this, -1, wxT("Camera FPS"));
    camFpsText->SetToolTip("Set the fps of the camera");
    fgs->Add(camFpsText);
    fgs->Add(camFpsField);
    wxStaticText* camWidthText = new wxStaticText(this, -1, wxT("Camera width in pixels"));
    camWidthText->SetToolTip("Width and height should be fine on 0, but change it to the camera resolution in case camera doesn't work correctly.");
    fgs->Add(camWidthText);
    fgs->Add(camWidthField);
    wxStaticText* camHeightText = new wxStaticText(this, -1, wxT("Camera height in pixels"));
    camHeightText->SetToolTip("Width and height should be fine on 0, but change it to the camera resolution in case camera doesn't work correctly.");
    fgs->Add(camHeightText);
    fgs->Add(camHeightField);
    wxStaticText* camLatencyText = new wxStaticText(this, -1, wxT("Camera latency"));
    camLatencyText->SetToolTip("Experimental. Should represent camera latency in seconds, but seems to work differently. Usually setting this to 1 shows good results.");
    fgs->Add(camLatencyText);
    fgs->Add(camLatencyField);
    //wxStaticText* cameraSettingsText = new wxStaticText(this, -1, wxT("Open camera settings"));
    //cameraSettingsText->SetToolTip("Experimental. Should open settings of your camera, but usually doesn't work. It might work for you");
    //fgs->Add(cameraSettingsText);
    //fgs->Add(cameraSettingsField);
    wxStaticText* chessboardCalibText = new wxStaticText(this, -1, wxT("Use chessboard calibration"));
    chessboardCalibText->SetToolTip("Use the old chessboard calibration. It is not recommended, but if you just have a chessboard and cant print a new board yet, you can check this.\n\n\
Keep other parameters as default unless you know what you are doing.");
    fgs->Add(chessboardCalibText);
    fgs->Add(chessboardCalibField);

    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("Hover over text for help!")));
    wxButton* btn1 = new wxButton(this, SAVE_BUTTON, "Save");
    //wxButton* btn2 = new wxButton(this, HELP_BUTTON, "Help");
    Connect(SAVE_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ParamsPage::SaveParams));
    //Connect(HELP_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ParamsPage::ShowHelp));
    fgs->Add(btn1);

    hbox->Add(fgs, 1, wxALL | wxEXPAND, 15);


    this->SetSizer(hbox);
}

void ParamsPage::ShowHelp(wxCommandEvent& event)
{
    wxMessageDialog dial(NULL, wxString::FromUTF8(
        "Short descriptions of main parameters \n\n"
        "Check the github for full tutorial and parameter descriptions!\n\n"
        "Parameters you have to set before starting:\n"
        "- Ip or ID of camera: will be a number 0-10 for USB cameras and \nhttp://'ip - here':8080/video for IP webcam\n"
        "- Number of trackers: set to 3 for full body. 2 will not work in vrchat!\n"
        "- Size of markers: Measure the white square on markers.\n"
        "- Quad decimate: can be 1, 1.5, 2, 3, 4. Higher values will increase FPS, but reduce maximum range of detections\n"
        "- Camera FPS, width, height: Set the fps. Width and height should be fine on 0, but change it in case camera doesn't work correctly.\n\n"
        "Other usefull parameters:\n"
        "- Rotate camera: Self explanatory. Use both for a 180° flip\n"
        "- Number of values for smoothing: Used to remove pose outliers. Can usually be lowered to 3 to reduce latency.\n"
        "- Additional smoothing: 0 to be fast, but very shaky, 1 to barely move the tracker, but smoothly. Experiment to find the sweet spot\n"
        "- Ignore tracker 0: If you want to replace the hip tracker with a vive tracker/owotrack, check this option. Keep number of trackers on 3.\n\n"
        "Experimental:\n"
        "- Camera latency: Increasing this value can help with camera latency. 1 seems to work best.\n"
        "- Use chessboard calibration: Use the old chessboard calibration. It is not recommended, but if you just have a chessboard and cant print a new board yet, you can check this.\n\n"
        "Keep other parameters as default unless you know what you are doing.\n\n"
        "Press OK to close this window."), wxT("Message"), wxOK);
    dial.ShowModal();
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
        //parameters->usePredictive = usePredictiveField->GetValue();
        //parameters->calibrationTracker = std::stoi(calibrationTrackerField->GetValue().ToStdString());
        parameters->ignoreTracker0 = ignoreTracker0Field->GetValue();
        parameters->rotateCl = rotateClField->GetValue();
        parameters->rotateCounterCl = rotateCounterClField->GetValue();
        //parameters->calibOffsetX = std::stod(offsetxField->GetValue().ToStdString());
        //parameters->calibOffsetY = std::stod(offsetyField->GetValue().ToStdString());
        //parameters->calibOffsetZ = std::stod(offsetzField->GetValue().ToStdString());
       // parameters->circularWindow = circularField->GetValue();
        parameters->smoothingFactor = std::stod(smoothingField->GetValue().ToStdString());
        parameters->camFps = std::stoi(camFpsField->GetValue().ToStdString());
        parameters->camWidth = std::stoi(camWidthField->GetValue().ToStdString());
        parameters->camHeight = std::stoi(camHeightField->GetValue().ToStdString());
        parameters->camLatency = std::stoi(camLatencyField->GetValue().ToStdString());
        //parameters->cameraSettings = cameraSettingsField->GetValue();
        parameters->chessboardCalib = chessboardCalibField->GetValue();
        parameters->Save();
        if (ignoreTracker0Field->GetValue() && std::stoi(trackerNumField->GetValue().ToStdString()) == 2)
        {
            wxMessageDialog dial(NULL,
                wxT("Number of trackers is 2 and ignore tracker 0 is on. This will result in only 1 tracker spawning in SteamVR. \nIf you wish to use both feet trackers, keep number of trackers at 3. \n\nParameters saved!"), wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }
        else
        {
            wxMessageDialog dial(NULL,
                wxT("Parameters saved!"), wxT("Info"), wxOK | wxICON_INFORMATION);
            dial.ShowModal();
        }
    }
    catch (std::exception&)
    {
        wxMessageDialog dial(NULL,
            wxT("Please enter appropriate values. Parameters were not saved."), wxT("Error"), wxOK | wxICON_ERROR);
        dial.ShowModal();
    }
}

ValueInput::ValueInput(wxPanel* parent, const wxString& nm, double val)
    : wxPanel(parent)
    , value(val)
    , input(new wxTextCtrl(this, 5, std::to_string(0)))
{
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ValueInput::ButtonPressed));
    Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ValueInput::ButtonPressed));
    Connect(wxEVT_MOUSEWHEEL, wxMouseEventHandler(ValueInput::MouseScroll));

    hbox->Add(new wxStaticText(this, -1, nm, wxDefaultPosition, wxSize(40, 20)));
    hbox->Add(new wxButton(this, 2, "<<", wxDefaultPosition, wxSize(25, 25)));
    hbox->Add(new wxButton(this, 1, "<", wxDefaultPosition, wxSize(25,25)));
    hbox->Add(input);
    hbox->Add(new wxButton(this, 3, ">", wxDefaultPosition, wxSize(25, 25)));
    hbox->Add(new wxButton(this, 4, ">>", wxDefaultPosition, wxSize(25, 25)));

    this->SetSizer(hbox);
}

void ValueInput::SetValue(double val)
{
    input->ChangeValue(std::to_string(val));
    value = val;
}

void ValueInput::MouseScroll(wxMouseEvent& evt)
{
    if (evt.GetWheelRotation() > 0)
    {
        value += 1;
    }
    else
    {
        value -= 1;
    }
    input->ChangeValue(std::to_string(value));
}

void ValueInput::ButtonPressed(wxCommandEvent &evt)
{
    int id = evt.GetId();
    if (id == 1)
    {
        value -= 1;
    }
    if (id == 2)
    {
        value -= 10;
    }
    if (id == 3)
    {
        value += 1;
    }
    if (id == 4)
    {
        value += 10;
    }
    if (id == 5)
    {
        try {
            value = std::stod(input->GetValue().ToStdString());
        }
        catch (std::exception&)
        {
        }
        return;
    }
    input->ChangeValue(std::to_string(value));
}
