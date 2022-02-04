#include "GUI.h"

#include <sstream>
#include <string>

#include <opencv2/videoio/registry.hpp>

namespace {

void addTextWithTooltip(wxWindow* parent, wxSizer* sizer, const wxString& label, const wxString& tooltip)
{
    wxStaticText* textObject = new wxStaticText(parent, -1, label);
    textObject->SetToolTip(tooltip);
    sizer->Add(textObject);
}

} // namespace

GUI::GUI(const wxString& title, Parameters * params, Connection* conn)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(650, 650))
{
    wxNotebook* nb = new wxNotebook(this, -1, wxPoint(-1, -1),
        wxSize(-1, -1), wxNB_TOP);

    CameraPage* panel = new CameraPage(nb,this,params);
    ParamsPage* panel2 = new ParamsPage(nb, params, conn);
    LicensePage* panel3 = new LicensePage(nb);

    nb->AddPage(panel, params->language.TAB_CAMERA);
    nb->AddPage(panel2, params->language.TAB_PARAMS);
    nb->AddPage(panel3, params->language.TAB_LICENSE);

    Centre();
}

LicensePage::LicensePage(wxNotebook* parent)
    :wxPanel(parent)
{
    wxNotebook* nb = new wxNotebook(this, -1, wxPoint(-1, -1),
        wxSize(-1, -1), wxNB_TOP);

    // Add 2 pages to the wxNotebook widget
    wxTextCtrl* textCtrl1 = new wxTextCtrl(nb, wxID_ANY, ATT_LICENSE, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    nb->AddPage(textCtrl1, "Tracking software");
    wxTextCtrl* textCtrl2 = new wxTextCtrl(nb, wxID_ANY, APRILTAG_LICENSE, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    nb->AddPage(textCtrl2, "Apriltag library");
    wxTextCtrl* textCtrl3 = new wxTextCtrl(nb, wxID_ANY, OPENCV_LICENSE, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    nb->AddPage(textCtrl3, "OpenCV");

    wxBoxSizer* bs = new wxBoxSizer(wxHORIZONTAL);
    bs->Add(nb,1, wxEXPAND);
    this->SetSizer(bs);


    //nb->AddPage(panel, "ATT");

    Centre();
}

CameraPage::CameraPage(wxNotebook* parent,GUI* parentGUI, Parameters* params)
    :wxPanel(parent)
{
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    wxFlexGridSizer* fgs = new wxFlexGridSizer(2, 20, 20);

    wxButton* btn1 = new wxButton(this, GUI::CAMERA_BUTTON, params->language.CAMERA_START_CAMERA);
    wxButton* btn2 = new wxButton(this, GUI::CAMERA_CALIB_BUTTON, params->language.CAMERA_CALIBRATE_CAMERA);
    wxButton* btn4 = new wxButton(this, GUI::CONNECT_BUTTON, params->language.CAMERA_CONNECT);
    wxButton* btn3 = new wxButton(this, GUI::TRACKER_CALIB_BUTTON, params->language.CAMERA_CALIBRATE_TRACKERS);
    wxButton* btn5 = new wxButton(this, GUI::START_BUTTON, params->language.CAMERA_START_DETECTION);

    wxCheckBox* cb1 = new wxCheckBox(this, GUI::CAMERA_CHECKBOX, params->language.CAMERA_PREVIEW_CAMERA,
        wxPoint(20, 20));
    wxCheckBox* cb2 = new wxCheckBox(this, GUI::CAMERA_CALIB_CHECKBOX, params->language.CAMERA_PREVIEW_CALIBRATION,
        wxPoint(20, 20));
    wxCheckBox* cb6 = new wxCheckBox(this, GUI::DISABLE_OUT_CHECKBOX, params->language.CAMERA_DISABLE_OUT,
        wxPoint(20, 20));
    //wxCheckBox* cb3 = new wxCheckBox(this, GUI::TIME_PROFILE_CHECKBOX, wxT("Show time profile"),
    //   wxPoint(20, 20));
    //parentGUI->cb2 = new wxCheckBox(this, GUI::SPACE_CALIB_CHECKBOX, wxT("Calibrate playspace"),
    //    wxPoint(20, 20));
    parentGUI->cb3 = new wxCheckBox(this, GUI::MANUAL_CALIB_CHECKBOX, params->language.CAMERA_CALIBRATION_MODE,
        wxPoint(20, 20));
    //parentGUI->cb2->SetValue(false);

    parentGUI->cb4 = new wxCheckBox(this, GUI::MULTICAM_AUTOCALIB_CHECKBOX, params->language.CAMERA_MULTICAM_CALIB,
        wxPoint(20, 20));

    parentGUI->cb5 = new wxCheckBox(this, GUI::LOCK_HEIGHT_CHECKBOX, params->language.CAMERA_LOCK_HEIGHT,
        wxPoint(20, 20));

    fgs->Add(btn1);
    fgs->Add(cb1);
    fgs->Add(btn2);
    fgs->Add(cb2);
    fgs->Add(btn3);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(new wxStaticText(this, -1, params->language.CAMERA_START_STEAMVR), 0, wxEXPAND);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn4);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn5);
    fgs->Add(parentGUI->cb3);
    fgs->Add(cb6);
    //fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    //fgs->Add(parentGUI->calibrationModeCheckbox);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(new wxStaticText(this, -1, ("Camera: " + params->octiuSah)), 0, wxEXPAND);
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

    parentGUI->posHbox->Add(new wxStaticText(this, -1, params->language.CAMERA_CALIBRATION_INSTRUCTION), 0, wxEXPAND);
    parentGUI->posHbox->Add(parentGUI->manualCalibX, 1, wxALL | wxEXPAND, 5);
    parentGUI->posHbox->Add(parentGUI->manualCalibY, 1, wxALL | wxEXPAND, 5);
    parentGUI->posHbox->Add(parentGUI->manualCalibZ, 1, wxALL | wxEXPAND, 5);
    parentGUI->posHbox->Add(parentGUI->manualCalibA, 1, wxALL | wxEXPAND, 5);
    parentGUI->posHbox->Add(parentGUI->manualCalibB, 1, wxALL | wxEXPAND, 5);
    parentGUI->posHbox->Add(parentGUI->manualCalibC, 1, wxALL | wxEXPAND, 5);
    parentGUI->posHbox->Add(parentGUI->cb4);
    parentGUI->posHbox->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    parentGUI->posHbox->Add(parentGUI->cb5);

    hbox->Add(parentGUI->posHbox, 1, wxALL | wxEXPAND, 15);
    //hbox->Add(parentGUI->rotHbox, 1, wxALL | wxEXPAND, 15);

    //hbox2->Show(false);

    this->SetSizer(hbox);

    //parentGUI->posHbox->Show(false);
    //parentGUI->rotHbox->Show(false);
}

ParamsPage::ParamsPage(wxNotebook* parent, Parameters* params, Connection* conn)
    : wxPanel(parent)
    , parameters(params)
    , connection(conn)
    , octiuSahField(new wxTextCtrl(this, -1, parameters->octiuSah)) //maru
    , cameraAddrField(new wxTextCtrl(this, -1, parameters->cameraAddr))
    , cameraApiField(new wxTextCtrl(this, -1, std::to_string(parameters->cameraApiPreference)))
    , trackerNumField(new wxTextCtrl(this, -1, std::to_string(parameters->trackerNum)))
    , markerSizeField(new wxTextCtrl(this, -1, std::to_string(parameters->markerSize * 100)))
    //, prevValuesField(new wxTextCtrl(this, -1, std::to_string(parameters->numOfPrevValues)))
    , smoothingField(new wxTextCtrl(this, -1, std::to_string(parameters->smoothingFactor)))
    , quadDecimateField(new wxTextCtrl(this, -1, std::to_string(parameters->quadDecimate)))
    , searchWindowField(new wxTextCtrl(this, -1, std::to_string(parameters->searchWindow)))
    // usePredictiveField(new wxCheckBox(this, -1, wxT("")))
    // calibrationTrackerField(new wxTextCtrl(this, -1, std::to_string(parameters->calibrationTracker)))
    , ignoreTracker0Field(new wxCheckBox(this, -1, wxT("")))
    , rotateClField(new wxCheckBox(this, -1, wxT("")))
    , rotateCounterClField(new wxCheckBox(this, -1, wxT("")))
    // offsetxField(new wxTextCtrl(this, -1, std::to_string(parameters->calibOffsetX)))
    // offsetyField(new wxTextCtrl(this, -1, std::to_string(parameters->calibOffsetY)))
    // offsetzField(new wxTextCtrl(this, -1, std::to_string(parameters->calibOffsetZ)))
    // circularField(new wxCheckBox(this, -1, wxT("")))
    , camFpsField(new wxTextCtrl(this, -1, std::to_string(parameters->camFps)))
    , camWidthField(new wxTextCtrl(this, -1, std::to_string(parameters->camWidth)))
    , camHeightField(new wxTextCtrl(this, -1, std::to_string(parameters->camHeight)))
    , camLatencyField(new wxTextCtrl(this, -1, std::to_string(parameters->camLatency)))
    , cameraSettingsField(new wxCheckBox(this, -1, wxT("")))
    , settingsParametersField(new wxCheckBox(this, -1, wxT("")))
    , cameraAutoexposureField(new wxTextCtrl(this, -1, std::to_string(parameters->cameraAutoexposure)))
    , cameraExposureField(new wxTextCtrl(this, -1, std::to_string(parameters->cameraExposure)))
    , cameraGainField(new wxTextCtrl(this, -1, std::to_string(parameters->cameraGain)))
    //, chessboardCalibField(new wxCheckBox(this, -1, wxT("")))
    , trackerCalibCentersField(new wxCheckBox(this, -1, wxT("")))
    , depthSmoothingField(new wxTextCtrl(this, -1, std::to_string(parameters->depthSmoothing)))
    , additionalSmoothingField(new wxTextCtrl(this, -1, std::to_string(parameters->additionalSmoothing)))
    
{
    //usePredictiveField->SetValue(parameters->usePredictive);
    ignoreTracker0Field->SetValue(parameters->ignoreTracker0);
    rotateClField->SetValue(parameters->rotateCl);
    rotateCounterClField->SetValue(parameters->rotateCounterCl);
    //circularField->SetValue(parameters->circularWindow);
    cameraSettingsField->SetValue(parameters->cameraSettings);
    //chessboardCalibField->SetValue(parameters->chessboardCalib);
    settingsParametersField->SetValue(parameters->settingsParameters);
    trackerCalibCentersField->SetValue(parameters->trackerCalibCenters);

    wxArrayString markerLibraryValues;
    markerLibraryValues.Add(wxT("ApriltagStandard"));
    markerLibraryValues.Add(wxT("ApriltagCircular"));
    markerLibraryValues.Add(wxT("Aruco4x4"));
    markerLibraryValues.Add(wxT("ApriltagColor"));

    markerLibraryField = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, markerLibraryValues);
    markerLibraryField->SetSelection(parameters->markerLibrary);

    wxArrayString languageValues;
    languageValues.Add(params->language.LANGUAGE_ENGLISH);
    languageValues.Add(params->language.LANGUAGE_CHINESE);

    languageField = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, languageValues);
    languageField->SetSelection(parameters->languageSelection);

    wxBoxSizer* hbox = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* fgs = new wxFlexGridSizer(4, 10, 10);

    addTextWithTooltip(this, fgs, params->language.PARAMS_LANGUAGE, params->language.PARAMS_LANGUAGE);
    fgs->Add(languageField);
    addTextWithTooltip(this, fgs, params->language.WINDOW_TITLE, params->language.WINDOW_TITLE_TOOLTIP);
    fgs->Add(octiuSahField);
    //fgs->Add(new wxStaticText(this, -1, wxT("")));
    //fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, params->language.PARAMS_CAMERA));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_ID, params->language.PARAMS_CAMERA_TOOLTIP_ID);
    fgs->Add(cameraAddrField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_API, params->language.PARAMS_CAMERA_TOOLTIP_API);
    fgs->Add(cameraApiField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_ROT_CLOCKWISE, params->language.PARAMS_CAMERA_TOOLTIP_ROT_CLOCKWISE);
    fgs->Add(rotateClField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_ROT_CCLOCKWISE, params->language.PARAMS_CAMERA_TOOLTIP_ROT_CCLOCKWISE);
    fgs->Add(rotateCounterClField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_WIDTH, params->language.PARAMS_CAMERA_TOOLTIP_WIDTH);
    fgs->Add(camWidthField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_HEIGHT, params->language.PARAMS_CAMERA_TOOLTIP_HEIGHT);
    fgs->Add(camHeightField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_FPS, params->language.PARAMS_CAMERA_TOOLTIP_FPS);
    fgs->Add(camFpsField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_SETTINGS, params->language.PARAMS_CAMERA_TOOLTIP_SETTINGS);
    fgs->Add(cameraSettingsField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_3_OPTIONS, params->language.PARAMS_CAMERA_TOOLTIP_3_OPTIONS);
    fgs->Add(settingsParametersField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_AUTOEXP, params->language.PARAMS_CAMERA_TOOLTIP_AUTOEXP);
    fgs->Add(cameraAutoexposureField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_EXP, params->language.PARAMS_CAMERA_TOOLTIP_EXP);
    fgs->Add(cameraExposureField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_CAMERA_NAME_GAIN, params->language.PARAMS_CAMERA_TOOLTIP_GAIN);
    fgs->Add(cameraGainField);

    fgs->Add(new wxStaticText(this, -1, params->language.PARAMS_TRACKER));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));

    addTextWithTooltip(this, fgs, params->language.PARAMS_TRACKER_NAME_NUM_TRACKERS, params->language.PARAMS_TRACKER_TOOLTIP_NUM_TRACKERS);
    fgs->Add(trackerNumField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_TRACKER_NAME_MARKER_SIZE, params->language.PARAMS_TRACKER_TOOLTIP_MARKER_SIZE);
    fgs->Add(markerSizeField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_TRACKER_NAME_QUAD_DECIMATE, params->language.PARAMS_TRACKER_TOOLTIP_QUAD_DECIMATE);
    fgs->Add(quadDecimateField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_TRACKER_NAME_SEARCH_WINDOW, params->language.PARAMS_TRACKER_TOOLTIP_SEARCH_WINDOW);
    fgs->Add(searchWindowField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_TRACKER_NAME_MARKER_LIBRARY, params->language.PARAMS_TRACKER_TOOLTIP_MARKER_LIBRARY);
    fgs->Add(markerLibraryField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_TRACKER_NAME_USE_CENTERS, params->language.PARAMS_TRACKER_TOOLTIP_USE_CENTERS);
    fgs->Add(trackerCalibCentersField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_TRACKER_NAME_IGNORE_0, params->language.PARAMS_TRACKER_TOOLTIP_IGNORE_0);
    fgs->Add(ignoreTracker0Field);

    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, params->language.PARAMS_SMOOTHING));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));

    //addTextWithTooltip(this, fgs, "Number of values for smoothing", "Used to remove pose outliers. Can usually be lowered to 3 to reduce latency.");
    //fgs->Add(prevValuesField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_SMOOTHING_NAME_WINDOW, params->language.PARAMS_SMOOTHING_TOOLTIP_WINDOW);
    fgs->Add(smoothingField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_SMOOTHING_NAME_ADDITIONAL, params->language.PARAMS_SMOOTHING_TOOLTIP_ADDITIONAL);
    fgs->Add(additionalSmoothingField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_SMOOTHING_NAME_DEPTH, params->language.PARAMS_SMOOTHING_TOOLTIP_DEPTH);
    fgs->Add(depthSmoothingField);
    addTextWithTooltip(this, fgs, params->language.PARAMS_SMOOTHING_NAME_CAM_LATENCY, params->language.PARAMS_SMOOTHING_TOOLTIP_CAM_LATENCY);
    fgs->Add(camLatencyField);


    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));

    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));

    fgs->Add(new wxStaticText(this, -1, params->language.PARAMS_HOVER_HELP));
    wxButton* btn1 = new wxButton(this, SAVE_BUTTON, params->language.PARAMS_SAVE);
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
        parameters->octiuSah = octiuSahField->GetValue().ToStdString();
        parameters->cameraAddr = cameraAddrField->GetValue().ToStdString();
        parameters->cameraApiPreference = std::stoi(cameraApiField->GetValue().ToStdString());
        parameters->trackerNum = std::stoi(trackerNumField->GetValue().ToStdString());
        parameters->markerSize = std::stod(markerSizeField->GetValue().ToStdString()) / 100;
        parameters->numOfPrevValues = 1; //std::stoi(prevValuesField->GetValue().ToStdString());
        parameters->quadDecimate = std::stod(quadDecimateField->GetValue().ToStdString());
        parameters->searchWindow = std::stod(searchWindowField->GetValue().ToStdString());
        //parameters->usePredictive = usePredictiveField->GetValue();
        //parameters->calibrationTracker = std::stoi(calibrationTrackerField->GetValue().ToStdString());
        parameters->ignoreTracker0 = ignoreTracker0Field->GetValue();
        parameters->rotateCl = rotateClField->GetValue();
        parameters->rotateCounterCl = rotateCounterClField->GetValue();
        //parameters->coloredMarkers = coloredMarkersField->GetValue();
        //parameters->calibOffsetX = std::stod(offsetxField->GetValue().ToStdString());
        //parameters->calibOffsetY = std::stod(offsetyField->GetValue().ToStdString());
        //parameters->calibOffsetZ = std::stod(offsetzField->GetValue().ToStdString());
       // parameters->circularWindow = circularField->GetValue();
        parameters->smoothingFactor = std::stod(smoothingField->GetValue().ToStdString());
        parameters->camFps = std::stoi(camFpsField->GetValue().ToStdString());
        parameters->camWidth = std::stoi(camWidthField->GetValue().ToStdString());
        parameters->camHeight = std::stoi(camHeightField->GetValue().ToStdString());
        parameters->camLatency = std::stod(camLatencyField->GetValue().ToStdString());
        parameters->cameraSettings = cameraSettingsField->GetValue();
        parameters->settingsParameters = settingsParametersField->GetValue();
        parameters->cameraAutoexposure = std::stoi(cameraAutoexposureField->GetValue().ToStdString());
        parameters->cameraExposure = std::stoi(cameraExposureField->GetValue().ToStdString());
        parameters->cameraGain = std::stoi(cameraGainField->GetValue().ToStdString());
        parameters->chessboardCalib = false;// chessboardCalibField->GetValue();
        parameters->trackerCalibCenters = trackerCalibCentersField->GetValue();
        parameters->depthSmoothing = std::stod(depthSmoothingField->GetValue().ToStdString());
        parameters->additionalSmoothing = std::stod(additionalSmoothingField->GetValue().ToStdString());
        parameters->markerLibrary = markerLibraryField->GetSelection();
        int prevLanguage = parameters->languageSelection;
        parameters->languageSelection = languageField->GetSelection();
        parameters->Save();

        if (parameters->depthSmoothing > 1)
            parameters->depthSmoothing = 1;
        if (parameters->depthSmoothing < 0)
            parameters->depthSmoothing = 0;

        if (parameters->languageSelection != prevLanguage)
        {
            wxMessageDialog dial(NULL,
                parameters->language.PARAMS_NOTE_LANGUAGECHANGE, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (parameters->smoothingFactor < 0.2)
        {
            wxMessageDialog dial(NULL,
                parameters->language.PARAMS_NOTE_LOW_SMOOTHING, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (parameters->quadDecimate != 1 && parameters->quadDecimate != 1.5 && parameters->quadDecimate != 2 && parameters->quadDecimate != 3 && parameters->quadDecimate != 4)
        {
            wxMessageDialog dial(NULL,
                parameters->language.PARAMS_NOTE_QUAD_NONSTANDARD, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (parameters->cameraSettings && parameters->cameraApiPreference != 700)
        {
            wxMessageDialog dial(NULL,
                parameters->language.PARAMS_NOTE_NO_DSHOW_CAMSETTINGS, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (parameters->smoothingFactor <= parameters->camLatency)
        {
            wxMessageDialog dial(NULL,
                parameters->language.PARAMS_NOTE_LATENCY_GREATER_SMOOTHING, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (parameters->smoothingFactor > 1)
        {
            wxMessageDialog dial(NULL,
                parameters->language.PARAMS_NOTE_HIGH_SMOOTHING, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (ignoreTracker0Field->GetValue() && std::stoi(trackerNumField->GetValue().ToStdString()) == 2)
        {
            wxMessageDialog dial(NULL,
                parameters->language.PARAMS_NOTE_2TRACKERS_IGNORE0, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        wxMessageDialog dial(NULL,
            parameters->language.PARAMS_SAVED_MSG, wxT("Info"), wxOK | wxICON_INFORMATION);
        dial.ShowModal();

        if (connection->status == connection->CONNECTED)
        {
            connection->Send("settings 120 " + std::to_string(parameters->smoothingFactor) + " " + std::to_string(parameters->additionalSmoothing));
        }
        
    }
    catch (std::exception&)
    {
        wxMessageDialog dial(NULL,
            parameters->language.PARAMS_WRONG_VALUES, wxT("Error"), wxOK | wxICON_ERROR);
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
