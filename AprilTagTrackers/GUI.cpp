#include "GUI.h"

#include "Debug.h"
#include "Localization.h"
#include "license.h"

#include <opencv2/highgui.hpp>
#include <opencv2/videoio/registry.hpp>

#include <algorithm>
#include <atomic>
#include <memory>
#include <sstream>
#include <string>

// Application icon in special c code format
#include "apriltag.xpm"

namespace
{

void addTextWithTooltip(wxWindow* parent, wxSizer* sizer, const wxString& label, const wxString& tooltip)
{
    wxStaticText* textObject = new wxStaticText(parent, -1, label);
    textObject->SetToolTip(tooltip);
    sizer->Add(textObject);
}

} // namespace

GUI::GUI(const wxString& title, Connection* conn, UserConfig& _userConfig, const Localization& lc)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(650, 650)),
      userConfig(_userConfig),
      previewEventLoop()
{
    wxNotebook* nb = new wxNotebook(this, -1, wxPoint(-1, -1),
        wxSize(-1, -1), wxNB_TOP);

    CameraPage* panel = new CameraPage(nb, this, lc);
    ParamsPage* panel2 = new ParamsPage(nb, conn, _userConfig, lc);
    LicensePage* panel3 = new LicensePage(nb);

    nb->AddPage(panel, lc.TAB_CAMERA);
    nb->AddPage(panel2, lc.TAB_PARAMS);
    nb->AddPage(panel3, lc.TAB_LICENSE);

    SetIcon(apriltag_xpm);

    Centre();

    Show();
}

void GUI::QueuePopup(const wxString& content, const wxString& caption, long style)
{
    // Queues the lambda to be executed on the wx gui thread, *eventually*
    // TODO: copy capture necessary?
    QueueOnGUIThread([=]()
        {
            wxMessageDialog dial(nullptr, content, caption, style);
            // ShowModal is blocking, until the user clicks ok
            dial.ShowModal(); //
        });
}

std::unique_ptr<PreviewWindow> GUI::CreatePreviewWindow(std::string title)
{
    static std::atomic_uint uniqueID = 0;
    std::stringstream windowID;
    windowID << "preview_gui_" << uniqueID++;

    if (!userConfig.windowTitle.empty())
        title += ": " + userConfig.windowTitle;

    return previewEventLoop.CreateWindow(std::move(windowID.str()), std::move(title));
}

LicensePage::LicensePage(wxNotebook* parent)
    : wxPanel(parent)
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
    bs->Add(nb, 1, wxEXPAND);
    this->SetSizer(bs);

    // nb->AddPage(panel, "ATT");

    Centre();
}

CameraPage::CameraPage(wxNotebook* parent, GUI* parentGUI, const Localization& lc)
    : wxPanel(parent)
{
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    wxFlexGridSizer* fgs = new wxFlexGridSizer(2, 20, 20);

    wxButton* btn1 = new wxButton(this, GUI::CAMERA_BUTTON, lc.CAMERA_START_CAMERA);
    wxButton* btn2 = new wxButton(this, GUI::CAMERA_CALIB_BUTTON, lc.CAMERA_CALIBRATE_CAMERA);
    wxButton* btn4 = new wxButton(this, GUI::CONNECT_BUTTON, lc.CAMERA_CONNECT);
    wxButton* btn3 = new wxButton(this, GUI::TRACKER_CALIB_BUTTON, lc.CAMERA_CALIBRATE_TRACKERS);
    wxButton* btn5 = new wxButton(this, GUI::START_BUTTON, lc.CAMERA_START_DETECTION);

    wxCheckBox* cb1 = new wxCheckBox(this, GUI::CAMERA_CHECKBOX, lc.CAMERA_PREVIEW_CAMERA, wxPoint(20, 20));
    wxCheckBox* cb2 = new wxCheckBox(this, GUI::CAMERA_CALIB_CHECKBOX, lc.CAMERA_PREVIEW_CALIBRATION, wxPoint(20, 20));
    wxCheckBox* cb6 = new wxCheckBox(this, GUI::DISABLE_OUT_CHECKBOX, lc.CAMERA_DISABLE_OUT, wxPoint(20, 20));
    wxCheckBox* cb7 = new wxCheckBox(this, GUI::DISABLE_OPENVR_API_CHECKBOX, lc.CAMERA_DISABLE_OPENVR_API, wxPoint(20, 20));
    // wxCheckBox* cb3 = new wxCheckBox(this, GUI::TIME_PROFILE_CHECKBOX, wxT("Show time profile"),
    //    wxPoint(20, 20));
    // parentGUI->cb2 = new wxCheckBox(this, GUI::SPACE_CALIB_CHECKBOX, wxT("Calibrate playspace"),
    //     wxPoint(20, 20));
    parentGUI->cb3 = new wxCheckBox(this, GUI::MANUAL_CALIB_CHECKBOX, lc.CAMERA_CALIBRATION_MODE,
        wxPoint(20, 20));
    // parentGUI->cb2->SetValue(false);

    parentGUI->cb4 = new wxCheckBox(this, GUI::MULTICAM_AUTOCALIB_CHECKBOX, lc.CAMERA_MULTICAM_CALIB,
        wxPoint(20, 20));

    parentGUI->cb5 = new wxCheckBox(this, GUI::LOCK_HEIGHT_CHECKBOX, lc.CAMERA_LOCK_HEIGHT,
        wxPoint(20, 20));

    cb7->SetValue(true);

    fgs->Add(btn1);
    fgs->Add(cb1);
    fgs->Add(btn2);
    fgs->Add(cb2);
    fgs->Add(btn3);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(new wxStaticText(this, -1, lc.CAMERA_START_STEAMVR), 0, wxEXPAND);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn4);
    fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    fgs->Add(btn5);
    fgs->Add(parentGUI->cb3);
    fgs->Add(cb6);
    fgs->Add(cb7);
    // fgs->Add(new wxStaticText(this, -1, wxT("")), 0, wxEXPAND);
    // fgs->Add(parentGUI->calibrationModeCheckbox);

    hbox->Add(fgs, 1, wxALL | wxEXPAND, 15);

    // hbox->Add(cb3, 1, wxALL | wxEXPAND, 15);

    parentGUI->posHbox = new wxBoxSizer(wxVERTICAL);
    parentGUI->rotHbox = new wxBoxSizer(wxVERTICAL);

    parentGUI->manualCalibX = new ValueInput(this, "X(cm):", 0);
    parentGUI->manualCalibY = new ValueInput(this, "Y(cm):", 0);
    parentGUI->manualCalibZ = new ValueInput(this, "Z(cm):", 0);
    parentGUI->manualCalibA = new ValueInput(this, wxString::FromUTF8("A(°):"), 0);
    parentGUI->manualCalibB = new ValueInput(this, wxString::FromUTF8("B(°):"), 0);
    parentGUI->manualCalibC = new ValueInput(this, wxString::FromUTF8("C(°):"), 0);

    parentGUI->posHbox->Add(new wxStaticText(this, -1, lc.CAMERA_CALIBRATION_INSTRUCTION), 0, wxEXPAND);
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
    // hbox->Add(parentGUI->rotHbox, 1, wxALL | wxEXPAND, 15);

    // hbox2->Show(false);

    this->SetSizer(hbox);

    // parentGUI->posHbox->Show(false);
    // parentGUI->rotHbox->Show(false);
}

ParamsPage::ParamsPage(wxNotebook* parent, Connection* conn, UserConfig& user_config, const Localization& _lc)
    : wxPanel(parent),
      connection(conn),
      user_config(user_config),
      lc(_lc),
      windowTitleField(new wxTextCtrl(this, -1, user_config.windowTitle)),
      cameraAddrField(new wxTextCtrl(this, -1, user_config.cameraAddr)),
      cameraApiField(new wxTextCtrl(this, -1, std::to_string(user_config.cameraApiPreference))),
      trackerNumField(new wxTextCtrl(this, -1, std::to_string(user_config.trackerNum))),
      markerSizeField(new wxTextCtrl(this, -1, std::to_string(user_config.markerSize * 100))),
      //, prevValuesField(new wxTextCtrl(this, -1, std::to_string(user_config.numOfPrevValues)))
      smoothingField(new wxTextCtrl(this, -1, std::to_string(user_config.smoothingFactor))),
      quadDecimateField(new wxTextCtrl(this, -1, std::to_string(user_config.quadDecimate))),
      searchWindowField(new wxTextCtrl(this, -1, std::to_string(user_config.searchWindow))),
      // usePredictiveField(new wxCheckBox(this, -1, wxT("")))
      // calibrationTrackerField(new wxTextCtrl(this, -1, std::to_string(user_config.calibrationTracker)))

      ignoreTracker0Field(new wxCheckBox(this, -1, wxT(""))), rotateClField(new wxCheckBox(this, -1, wxT(""))), rotateCounterClField(new wxCheckBox(this, -1, wxT(""))),
      // offsetxField(new wxTextCtrl(this, -1, std::to_string(user_config.calibOffsetX)))
      // offsetyField(new wxTextCtrl(this, -1, std::to_string(user_config.calibOffsetY)))
      // offsetzField(new wxTextCtrl(this, -1, std::to_string(user_config.calibOffsetZ)))
      // circularField(new wxCheckBox(this, -1, wxT("")))
      camFpsField(new wxTextCtrl(this, -1, std::to_string(user_config.camFps))),
      camWidthField(new wxTextCtrl(this, -1, std::to_string(user_config.camWidth))),
      camHeightField(new wxTextCtrl(this, -1, std::to_string(user_config.camHeight))),
      camLatencyField(new wxTextCtrl(this, -1, std::to_string(user_config.camLatency))),
      cameraSettingsField(new wxCheckBox(this, -1, wxT(""))),
      settingsParametersField(new wxCheckBox(this, -1, wxT(""))),
      cameraAutoexposureField(new wxTextCtrl(this, -1, std::to_string(user_config.cameraAutoexposure))),
      cameraExposureField(new wxTextCtrl(this, -1, std::to_string(user_config.cameraExposure))),
      cameraGainField(new wxTextCtrl(this, -1, std::to_string(user_config.cameraGain))),
      //, chessboardCalibField(new wxCheckBox(this, -1, wxT("")))

      trackerCalibCentersField(new wxCheckBox(this, -1, wxT(""))),
      depthSmoothingField(new wxTextCtrl(this, -1, std::to_string(user_config.depthSmoothing))),
      additionalSmoothingField(new wxTextCtrl(this, -1, std::to_string(user_config.additionalSmoothing)))
{
    // usePredictiveField->SetValue(user_config.usePredictive);
    ignoreTracker0Field->SetValue(user_config.ignoreTracker0);
    rotateClField->SetValue(user_config.rotateCl);
    rotateCounterClField->SetValue(user_config.rotateCounterCl);
    // circularField->SetValue(user_config.circularWindow);
    cameraSettingsField->SetValue(user_config.cameraSettings);
    // chessboardCalibField->SetValue(user_config.chessboardCalib);
    settingsParametersField->SetValue(user_config.settingsParameters);
    trackerCalibCentersField->SetValue(user_config.trackerCalibCenters);

    wxArrayString markerLibraryValues;
    markerLibraryValues.Add(wxT("ApriltagStandard"));
    markerLibraryValues.Add(wxT("ApriltagCircular"));
    markerLibraryValues.Add(wxT("Aruco4x4"));
    markerLibraryValues.Add(wxT("ApriltagColor"));

    markerLibraryField = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, markerLibraryValues);
    markerLibraryField->SetSelection(user_config.markerLibrary);

    wxArrayString languageValues{Localization::LANG_NAME_MAP.size(), Localization::LANG_NAME_MAP.data()};

    languageField = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, languageValues);
    languageField->SetSelection(Localization::LangCodeToIndex(user_config.langCode));

    wxBoxSizer* hbox = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* fgs = new wxFlexGridSizer(4, 10, 10);

    addTextWithTooltip(this, fgs, lc.PARAMS_LANGUAGE, lc.PARAMS_LANGUAGE);
    fgs->Add(languageField);
    addTextWithTooltip(this, fgs, lc.WINDOW_TITLE, lc.WINDOW_TITLE_TOOLTIP);
    fgs->Add(windowTitleField);

    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, lc.PARAMS_CAMERA));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));

    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_ID, lc.PARAMS_CAMERA_TOOLTIP_ID);
    fgs->Add(cameraAddrField);

    std::stringstream cameraTooltipApi;
    cameraTooltipApi << lc.PARAMS_CAMERA_TOOLTIP_API_1;
    for (const auto& backend : cv::videoio_registry::getCameraBackends())
    {
        cameraTooltipApi << "\n"
                         << static_cast<int>(backend)
                         << ": " << cv::videoio_registry::getBackendName(backend);
    }
    cameraTooltipApi << "\n\n"
                     << lc.PARAMS_CAMERA_TOOLTIP_API_2;
    for (const auto& backend : cv::videoio_registry::getStreamBackends())
    {
        cameraTooltipApi << "\n"
                         << static_cast<int>(backend)
                         << ": " << cv::videoio_registry::getBackendName(backend);
    }

    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_API, cameraTooltipApi.str());
    fgs->Add(cameraApiField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_ROT_CLOCKWISE, lc.PARAMS_CAMERA_TOOLTIP_ROT_CLOCKWISE);
    fgs->Add(rotateClField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_ROT_CCLOCKWISE, lc.PARAMS_CAMERA_TOOLTIP_ROT_CCLOCKWISE);
    fgs->Add(rotateCounterClField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_WIDTH, lc.PARAMS_CAMERA_TOOLTIP_WIDTH);
    fgs->Add(camWidthField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_HEIGHT, lc.PARAMS_CAMERA_TOOLTIP_HEIGHT);
    fgs->Add(camHeightField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_FPS, lc.PARAMS_CAMERA_TOOLTIP_FPS);
    fgs->Add(camFpsField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_SETTINGS, lc.PARAMS_CAMERA_TOOLTIP_SETTINGS);
    fgs->Add(cameraSettingsField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_3_OPTIONS, lc.PARAMS_CAMERA_TOOLTIP_3_OPTIONS);
    fgs->Add(settingsParametersField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_AUTOEXP, lc.PARAMS_CAMERA_TOOLTIP_AUTOEXP);
    fgs->Add(cameraAutoexposureField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_EXP, lc.PARAMS_CAMERA_TOOLTIP_EXP);
    fgs->Add(cameraExposureField);
    addTextWithTooltip(this, fgs, lc.PARAMS_CAMERA_NAME_GAIN, lc.PARAMS_CAMERA_TOOLTIP_GAIN);
    fgs->Add(cameraGainField);

    fgs->Add(new wxStaticText(this, -1, lc.PARAMS_TRACKER));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));

    addTextWithTooltip(this, fgs, lc.PARAMS_TRACKER_NAME_NUM_TRACKERS, lc.PARAMS_TRACKER_TOOLTIP_NUM_TRACKERS);
    fgs->Add(trackerNumField);
    addTextWithTooltip(this, fgs, lc.PARAMS_TRACKER_NAME_MARKER_SIZE, lc.PARAMS_TRACKER_TOOLTIP_MARKER_SIZE);
    fgs->Add(markerSizeField);
    addTextWithTooltip(this, fgs, lc.PARAMS_TRACKER_NAME_QUAD_DECIMATE, lc.PARAMS_TRACKER_TOOLTIP_QUAD_DECIMATE);
    fgs->Add(quadDecimateField);
    addTextWithTooltip(this, fgs, lc.PARAMS_TRACKER_NAME_SEARCH_WINDOW, lc.PARAMS_TRACKER_TOOLTIP_SEARCH_WINDOW);
    fgs->Add(searchWindowField);
    addTextWithTooltip(this, fgs, lc.PARAMS_TRACKER_NAME_MARKER_LIBRARY, lc.PARAMS_TRACKER_TOOLTIP_MARKER_LIBRARY);
    fgs->Add(markerLibraryField);
    addTextWithTooltip(this, fgs, lc.PARAMS_TRACKER_NAME_USE_CENTERS, lc.PARAMS_TRACKER_TOOLTIP_USE_CENTERS);
    fgs->Add(trackerCalibCentersField);
    addTextWithTooltip(this, fgs, lc.PARAMS_TRACKER_NAME_IGNORE_0, lc.PARAMS_TRACKER_TOOLTIP_IGNORE_0);
    fgs->Add(ignoreTracker0Field);

    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, lc.PARAMS_SMOOTHING));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));

    // addTextWithTooltip(this, fgs, "Number of values for smoothing", "Used to remove pose outliers. Can usually be lowered to 3 to reduce latency.");
    // fgs->Add(prevValuesField);
    addTextWithTooltip(this, fgs, lc.PARAMS_SMOOTHING_NAME_WINDOW, lc.PARAMS_SMOOTHING_TOOLTIP_WINDOW);
    fgs->Add(smoothingField);
    addTextWithTooltip(this, fgs, lc.PARAMS_SMOOTHING_NAME_ADDITIONAL, lc.PARAMS_SMOOTHING_TOOLTIP_ADDITIONAL);
    fgs->Add(additionalSmoothingField);
    addTextWithTooltip(this, fgs, lc.PARAMS_SMOOTHING_NAME_DEPTH, lc.PARAMS_SMOOTHING_TOOLTIP_DEPTH);
    fgs->Add(depthSmoothingField);
    addTextWithTooltip(this, fgs, lc.PARAMS_SMOOTHING_NAME_CAM_LATENCY, lc.PARAMS_SMOOTHING_TOOLTIP_CAM_LATENCY);
    fgs->Add(camLatencyField);

    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));

    fgs->Add(new wxStaticText(this, -1, wxT("")));
    fgs->Add(new wxStaticText(this, -1, wxT("")));

    fgs->Add(new wxStaticText(this, -1, lc.PARAMS_HOVER_HELP));
    wxButton* btn1 = new wxButton(this, SAVE_BUTTON, lc.PARAMS_SAVE);
    // wxButton* btn2 = new wxButton(this, HELP_BUTTON, "Help");
    Connect(SAVE_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ParamsPage::SaveParams));
    // Connect(HELP_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ParamsPage::ShowHelp));
    fgs->Add(btn1);

    hbox->Add(fgs, 1, wxALL | wxEXPAND, 15);

    this->SetSizer(hbox);
}

void ParamsPage::SaveParams(wxCommandEvent& event)
{
    try
    {
        user_config.windowTitle = windowTitleField->GetValue();
        user_config.cameraAddr = cameraAddrField->GetValue().ToStdString();
        user_config.cameraApiPreference = std::stoi(cameraApiField->GetValue().ToStdString());
        user_config.trackerNum = std::stoi(trackerNumField->GetValue().ToStdString());
        user_config.markerSize = std::stod(markerSizeField->GetValue().ToStdString()) / 100;
        user_config.numOfPrevValues = 1; // std::stoi(prevValuesField->GetValue().ToStdString());
        user_config.quadDecimate = std::stod(quadDecimateField->GetValue().ToStdString());
        user_config.searchWindow = std::stod(searchWindowField->GetValue().ToStdString());
        // user_config.usePredictive = usePredictiveField->GetValue();
        // user_config.calibrationTracker = std::stoi(calibrationTrackerField->GetValue().ToStdString());
        user_config.ignoreTracker0 = ignoreTracker0Field->GetValue();
        user_config.rotateCl = rotateClField->GetValue();
        user_config.rotateCounterCl = rotateCounterClField->GetValue();
        // user_config.coloredMarkers = coloredMarkersField->GetValue();
        // user_config.calibOffsetX = std::stod(offsetxField->GetValue().ToStdString());
        // user_config.calibOffsetY = std::stod(offsetyField->GetValue().ToStdString());
        // user_config.calibOffsetZ = std::stod(offsetzField->GetValue().ToStdString());
        // user_config.circularWindow = circularField->GetValue();
        user_config.smoothingFactor = std::stod(smoothingField->GetValue().ToStdString());
        user_config.camFps = std::stoi(camFpsField->GetValue().ToStdString());
        user_config.camWidth = std::stoi(camWidthField->GetValue().ToStdString());
        user_config.camHeight = std::stoi(camHeightField->GetValue().ToStdString());
        user_config.camLatency = std::stod(camLatencyField->GetValue().ToStdString());
        user_config.cameraSettings = cameraSettingsField->GetValue();
        user_config.settingsParameters = settingsParametersField->GetValue();
        user_config.cameraAutoexposure = std::stoi(cameraAutoexposureField->GetValue().ToStdString());
        user_config.cameraExposure = std::stoi(cameraExposureField->GetValue().ToStdString());
        user_config.cameraGain = std::stoi(cameraGainField->GetValue().ToStdString());
        user_config.chessboardCalib = false; // chessboardCalibField->GetValue();
        user_config.trackerCalibCenters = trackerCalibCentersField->GetValue();
        user_config.depthSmoothing = std::stof(depthSmoothingField->GetValue().ToStdString());
        user_config.additionalSmoothing = std::stof(additionalSmoothingField->GetValue().ToStdString());
        user_config.markerLibrary = markerLibraryField->GetSelection();
        auto prevLanguage = user_config.langCode;
        user_config.langCode = Localization::LANG_CODE_MAP.at(languageField->GetSelection());
        user_config.Save();

        if (user_config.depthSmoothing > 1)
            user_config.depthSmoothing = 1;
        if (user_config.depthSmoothing < 0)
            user_config.depthSmoothing = 0;

        if (user_config.langCode != prevLanguage)
        {
            wxMessageDialog dial(NULL,
                lc.PARAMS_NOTE_LANGUAGECHANGE, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (user_config.smoothingFactor < 0.2)
        {
            wxMessageDialog dial(NULL,
                lc.PARAMS_NOTE_LOW_SMOOTHING, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (user_config.quadDecimate != 1 && user_config.quadDecimate != 1.5 && user_config.quadDecimate != 2 && user_config.quadDecimate != 3 && user_config.quadDecimate != 4)
        {
            wxMessageDialog dial(NULL,
                lc.PARAMS_NOTE_QUAD_NONSTANDARD, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (user_config.cameraSettings && user_config.cameraApiPreference != 700)
        {
            wxMessageDialog dial(NULL,
                lc.PARAMS_NOTE_NO_DSHOW_CAMSETTINGS, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (user_config.smoothingFactor <= user_config.camLatency)
        {
            wxMessageDialog dial(NULL,
                lc.PARAMS_NOTE_LATENCY_GREATER_SMOOTHING, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (user_config.smoothingFactor > 1)
        {
            wxMessageDialog dial(NULL,
                lc.PARAMS_NOTE_HIGH_SMOOTHING, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        if (ignoreTracker0Field->GetValue() && std::stoi(trackerNumField->GetValue().ToStdString()) == 2)
        {
            wxMessageDialog dial(NULL,
                lc.PARAMS_NOTE_2TRACKERS_IGNORE0, wxT("Warning"), wxOK | wxICON_WARNING);
            dial.ShowModal();
        }

        wxMessageDialog dial(NULL,
            lc.PARAMS_SAVED_MSG, wxT("Info"), wxOK | wxICON_INFORMATION);
        dial.ShowModal();

        if (connection->status == connection->CONNECTED)
        {
            connection->Send("settings 120 " + std::to_string(user_config.smoothingFactor) + " " + std::to_string(user_config.additionalSmoothing));
        }
    }
    catch (const std::exception& e)
    {
        ATERROR("Caught: " << e.what());
        wxMessageDialog dial(NULL,
            lc.PARAMS_WRONG_VALUES, wxT("Error"), wxOK | wxICON_ERROR);
        dial.ShowModal();
    }
}

ValueInput::ValueInput(wxPanel* parent, const wxString& nm, double val)
    : wxPanel(parent), value(val), input(new wxTextCtrl(this, 5, std::to_string(0)))
{
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ValueInput::ButtonPressed));
    Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ValueInput::ButtonPressed));
    Connect(wxEVT_MOUSEWHEEL, wxMouseEventHandler(ValueInput::MouseScroll));

    hbox->Add(new wxStaticText(this, -1, nm, wxDefaultPosition, wxSize(40, 20)));
    hbox->Add(new wxButton(this, 2, "<<", wxDefaultPosition, wxSize(25, 25)));
    hbox->Add(new wxButton(this, 1, "<", wxDefaultPosition, wxSize(25, 25)));
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

void ValueInput::ButtonPressed(wxCommandEvent& evt)
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
        try
        {
            value = std::stod(input->GetValue().ToStdString());
        }
        catch (std::exception&)
        {
        }
        return;
    }
    input->ChangeValue(std::to_string(value));
}

namespace
{

/// Checks if opencv internally has a window with id.
/// Can be called from any thread
inline bool CVIsVisible(const std::string& id)
{
    ATASSERT("Non-empty window ID.", !id.empty());
    // Returns a double, -1 if window not found, property always returns 1.
    // We never set the visible property, so it should always be true, seems like the default behaviour
    // This isn't a settable property, so no way to "hide" a window
    return cv::getWindowProperty(id, cv::WindowPropertyFlags::WND_PROP_VISIBLE) > 0;
}

/// Create a window and update its title
inline void CVCreateWindow(const std::string& id, const std::string& title = "")
{
    ATASSERT("Non-empty window ID.", !id.empty());
    ATASSERT("Call GUI function on main thread.", Debug::IsMainThread());
    ATASSERT("Window with unique ID does not exist.", !CVIsVisible(id));
    ATTRACE("CVCreateWindow: " << id);
    // create an empty named window instance
    cv::namedWindow(id);
    // Empty title, original title will be id instead
    if (!title.empty())
        cv::setWindowTitle(id, title);
}

inline void CVDestroyWindow(const std::string& id)
{
    ATASSERT("Non-empty window ID.", !id.empty());
    ATASSERT("Call GUI function on main thread.", Debug::IsMainThread());
    ATASSERT("destroyWindow will throw if window with ID does not exist.", CVIsVisible(id));
    ATTRACE("CVDestroyWindow: " << id);
    cv::destroyWindow(id);
}

inline void CVShowImage(const std::string& id, const cv::Mat& image)
{
    ATASSERT("Non-empty window ID.", !id.empty());
    ATASSERT("Call GUI function on main thread.", Debug::IsMainThread());
    ATASSERT("Window is created by namedWindow instead of imshow.", CVIsVisible(id));
    cv::imshow(id, image);
}

} // namespace

PreviewWindow::PreviewWindow(std::string _id, std::string _title, PreviewEventLoop& _parentEventLoop)
    : id(std::move(_id)), title(std::move(_title)), parentEventLoop(_parentEventLoop)
{
    ATASSERT("Unique window id does not exist yet.", !IsVisible());
}

PreviewWindow::~PreviewWindow()
{
    parentEventLoop.DestroyWindow(*this);
}

bool PreviewWindow::IsVisible() const
{
    return CVIsVisible(this->id);
}

void PreviewWindow::CloneSetImage(const cv::Mat& newImage)
{
    const auto lock = std::lock_guard(imageMutex);
    newImageReady = true;
    newImage.copyTo(image);
}

void PreviewWindow::SwapSetImage(cv::Mat& newImage)
{
    const auto lock = std::lock_guard(imageMutex);
    newImageReady = true;
    cv::swap(image, newImage);
}

void PreviewWindow::CreateOrUpdateWindow() const
{
    const auto lock = std::lock_guard(imageMutex);
    // if SetImage has not been called yet
    if (!newImageReady || image.empty()) return;

    if (!IsVisible())
        CVCreateWindow(this->id, this->title);

    CVShowImage(this->id, this->image);
}

void PreviewWindow::Hide()
{
    if (!IsVisible()) return;

    parentEventLoop.QueueOnGUIThread(
        [this]()
        {
            const auto lock = std::lock_guard(imageMutex);
            newImageReady = false;
            if (CVIsVisible(id)) CVDestroyWindow(id);
        });
}

auto PreviewEventLoop::FindWindowByID(const std::string& id)
{
    return std::find_if(windowList.begin(), windowList.end(),
        [&](const auto& elem)
        {
            return elem.get().id == id;
        });
}

std::unique_ptr<PreviewWindow> PreviewEventLoop::CreateWindow(std::string id, std::string title)
{
    const auto lock = std::lock_guard(windowListMutex);

    ATASSERT("Window with ID " << id << " is destroyed before being created again.",
        FindWindowByID(id) == windowList.end());

    auto preview = std::unique_ptr<PreviewWindow>(new PreviewWindow(std::move(id), std::move(title), *this));
    windowList.emplace_back(*preview.get());
    return preview;
}

void PreviewEventLoop::DestroyWindow(PreviewWindow& window)
{
    {
        const auto lock = std::lock_guard(windowListMutex);
        const auto& itr = FindWindowByID(window.id);

        ATASSERT("Window was created by this instance.",
            itr != windowList.end());

        windowList.erase(itr);
    }

    QueueOnGUIThread(
        [windowID = window.id]()
        {
            if (CVIsVisible(windowID))
                CVDestroyWindow(windowID);
        });
}

// cv::pollKey some how processes the next timer event, causing Notify to be recursive
// Can't call pollKey from within wxWidgets event handlers.
void PreviewEventLoop::Notify()
{
    if (isRecursiveNotifyCall)
    {
        ATTRACE("Recursive notify call.");
        return;
    }
    isRecursiveNotifyCall = true;

    const auto lock = std::lock_guard(windowListMutex);
    bool anyWindowVisible = false;
    for (const PreviewWindow& window : windowList)
    {
        window.CreateOrUpdateWindow();
        anyWindowVisible |= window.IsVisible();
    }
    if (anyWindowVisible)
        cv::pollKey();

    isRecursiveNotifyCall = false;
}
