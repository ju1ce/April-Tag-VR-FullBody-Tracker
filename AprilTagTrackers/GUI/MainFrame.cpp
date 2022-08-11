#include "MainFrame.hpp"

#include "license.hpp"
#include "wxHelpers.hpp"

#include <functional>
#include <sstream>

GUI::MainFrame::MainFrame(RefPtr<ITrackerControl> _tracker, const Localization& _lc, UserConfig& _config)
    : wxFrame(nullptr, wxID_ANY, _lc.APP_TITLE),
      tracker(_tracker), lc(_lc), config(_config),
      previews{PreviewFrame{"Preview [" + _config.windowTitle + "]"}, PreviewFrame{"Camera Preview [" + _config.windowTitle + "]"}}
{
    SetIcon(apriltag_xpm);
    if (!config.windowTitle.empty()) SetTitle(config.windowTitle);

    statusBar = NewWindow<wxStatusBar>(static_cast<wxFrame*>(this));
    SetStatusBar(statusBar);
    statusBar->SetFieldsCount(static_cast<int>(StatusItem::COUNT));
    SetStatus(false, StatusItem::Camera);
    SetStatus(false, StatusItem::Driver);
    SetStatus(false, StatusItem::Tracker);

    auto topSizer = NewSizer<wxBoxSizer>(static_cast<wxFrame*>(this), wxHORIZONTAL);
    auto pages = NewWindow<wxNotebook>(static_cast<wxFrame*>(this), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
    // proportion of 1 means it will expand horizontally in the box sizer
    topSizer->Add(pages, wxSizerFlags(1).Expand());

    CreateCameraPage(pages);
    CreateParamsPage(pages);
    CreateLicensePage(pages);

    Layout();
    Fit();
    Show();

    Bind(wxEVT_CLOSE_WINDOW, [this](auto&)
        {
            tracker->Stop();
            Destroy();
        });

    Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, MemberFunc(&MainFrame::OnNotebookPageChanged, this));
}

void GUI::MainFrame::ShowPopup(const U8String& msg, PopupStyle style)
{
    U8String caption;
    int icon = 0;
    if (style == PopupStyle::Error)
    {
        caption = lc.word.Error;
        icon = wxICON_ERROR;
    }
    else if (style == PopupStyle::Warning)
    {
        caption = lc.word.Warning;
        icon = wxICON_WARNING;
    }
    else if (style == PopupStyle::Info)
    {
        caption = lc.word.Info;
        icon = wxICON_INFORMATION;
    }
    wxMessageDialog dial(nullptr, msg, caption, wxOK | icon);
    dial.ShowModal();
}

void GUI::MainFrame::SetStatus(bool status, StatusItem item)
{
    const int index = static_cast<int>(item);

    U8String text;
    if (item == StatusItem::Camera) text = lc.status.CAMERA;
    else if (item == StatusItem::Driver) text = lc.status.DRIVER;
    else if (item == StatusItem::Tracker) text = lc.status.TRACKER;

    text += status ? lc.word.On : lc.word.Off;
    statusBar->SetStatusText(text, index);
}

void GUI::MainFrame::ShowPrompt(const U8String& msg, const std::function<void(bool)>& onClose)
{
    wxMessageDialog dial(nullptr, msg, "Message", wxOK | wxCANCEL);
    // ShowModal is blocking, until the user interacts
    int response = dial.ShowModal();
    onClose(response == wxID_OK);
}

void GUI::MainFrame::UpdatePreview(const cv::Mat& image, PreviewId id)
{
    int idx = static_cast<int>(id);
    previews[idx].UpdateImage(image);
}

bool GUI::MainFrame::IsPreviewVisible(PreviewId id)
{
    int idx = static_cast<int>(id);
    return previews[idx].IsVisible();
}

ManualCalib::Real GUI::MainFrame::GetManualCalib()
{
    return config.manualCalib.GetAsReal();
}

void GUI::MainFrame::SetManualCalib(const ManualCalib::Real& calib)
{
    config.manualCalib.SetFromReal(calib);

    using Clock = std::chrono::steady_clock;
    static Clock::time_point lastUpdate = Clock::now();
    if ((Clock::now() - lastUpdate) > std::chrono::milliseconds(500))
    {
        lastUpdate = Clock::now();
        CallOnMainThread([this]
            {
                manualCalibForm->Update();
            });
    }
}

void GUI::MainFrame::SetManualCalibVisible(bool visible)
{
    manualCalibForm->SetSizerVisible(visible);
    // can be called from checkbox event, SetValue won't trigger another event
    manualCalibCheckBox->SetValue(visible);
    if (visible)
    {
        manualCalibForm->Update();
        Fit();
    }
    else 
    {
        config.Save();
    }
    tracker->manualRecalibrate = visible;
}

void GUI::MainFrame::OnNotebookPageChanged(wxBookCtrlEvent& evt)
{
    RefPtr<wxNotebook> nb = static_cast<wxNotebook*>(evt.GetEventObject());
    wxWindow* page = nb->GetCurrentPage();
    if (page == nullptr) return;
    wxSizer* sizer = page->GetSizer();
    if (sizer == nullptr) return;
    Fit();
}

void GUI::MainFrame::SetPreviewVisible(bool visible, PreviewId id, bool userCanDestroy)
{
    int idx = static_cast<int>(id);
    if (visible)
    {
        if (userCanDestroy)
            previews[idx].Show(PreviewFrame::CloseButton::Show);
        else
            previews[idx].Show(PreviewFrame::CloseButton::Hide);
    }
    else
    {
        previews[idx].Hide();
    }
}

void GUI::MainFrame::SaveParams()
{
    params.Submit();
    config.Save();
    CallAfter([&]
        {
            ValidateParams();
        });
}

void GUI::MainFrame::ValidateParams()
{
    if (config.smoothingFactor < 0.2)
        ShowPopup(lc.PARAMS_NOTE_LOW_SMOOTHING, PopupStyle::Warning);
    if (config.cameraSettings && config.cameraApiPreference != 700)
        ShowPopup(lc.PARAMS_NOTE_NO_DSHOW_CAMSETTINGS, PopupStyle::Warning);
    if (config.smoothingFactor <= config.camLatency)
        ShowPopup(lc.PARAMS_NOTE_LATENCY_GREATER_SMOOTHING, PopupStyle::Warning);
    if (config.smoothingFactor > 1)
        ShowPopup(lc.PARAMS_NOTE_HIGH_SMOOTHING, PopupStyle::Warning);
    if (config.ignoreTracker0 && config.trackerNum == 2)
        ShowPopup(lc.PARAMS_NOTE_2TRACKERS_IGNORE0, PopupStyle::Warning);

    tracker->UpdateConfig();
    ShowPopup(lc.PARAMS_SAVED_MSG, PopupStyle::Info);
}

void GUI::MainFrame::CreateCameraPage(RefPtr<wxNotebook> pages)
{
    using namespace Form;

    auto panel = NewWindow<wxPanel>(pages);
    pages->AddPage(panel, lc.TAB_CAMERA, true);

    auto boxSizer = NewSizer<wxBoxSizer>(panel, wxHORIZONTAL);
    cam = FormBuilder(panel, boxSizer);

    cam.PushSizer<wxFlexGridSizer>(2, wxSize(20, 10))
        .Border(wxALL, 5)
        .Add(Button{lc.CAMERA_START_CAMERA, [this](auto&)
            {
                tracker->StartCamera();
            }})
        .Add(Button{lc.CAMERA_PREVIEW_CAMERA, [this](auto&)
            {
                previews[static_cast<int>(PreviewId::Camera)].Show();
            }})
        .Add(Button{lc.CAMERA_CALIBRATE_CAMERA, [this](auto&)
            {
                tracker->StartCameraCalib();
            }})
        .Add(CheckBoxButton{lc.CAMERA_PREVIEW_CALIBRATION, [this](auto& evt)
            {
                tracker->previewCameraCalibration = evt.IsChecked();
            }})
        .Add(Button{lc.CAMERA_CALIBRATE_TRACKERS, [this](auto&)
            {
                tracker->StartTrackerCalib();
            }})
        .Add(StretchSpacer{})
        .Add(Label{lc.CAMERA_START_STEAMVR})
        .Add(CheckBoxButton{lc.CAMERA_DISABLE_OPENVR_API, [this](auto& evt)
            {
                config.disableOpenVrApi = evt.IsChecked();
                config.Save();
            }})
        .Add(Button{lc.CAMERA_CONNECT, [this](auto&)
            {
                tracker->StartConnection();
            }})
        .Add(StretchSpacer{})
        .Add(Button{lc.CAMERA_START_DETECTION, [this](auto&)
            {
                tracker->Start();
            }})
        .Add(Button{"Preview output", [this](auto& evt)
            {
                SetPreviewVisible(true);
            }});

    manualCalibCheckBox =
        cam.AddGet(CheckBoxButton{lc.CAMERA_CALIBRATION_MODE,
                       [this](auto& evt)
                       {
                           SetManualCalibVisible(evt.IsChecked());
                       }})
            ->GetWidget();

    manualCalibForm = cam.PopSizer().SubForm();

    manualCalibForm->PushSizer<wxBoxSizer>(wxVERTICAL)
        .Add(Label{lc.CAMERA_CALIBRATION_INSTRUCTION})
        .Add(Labeled{lc.calib.X, InputNumber{config.manualCalib.posOffset[0]}})
        .Add(Labeled{lc.calib.Y, InputNumber{config.manualCalib.posOffset[1]}})
        .Add(Labeled{lc.calib.Z, InputNumber{config.manualCalib.posOffset[2]}})
        .Add(Labeled{lc.calib.PITCH, InputNumber{config.manualCalib.angleOffset[0]}})
        .Add(Labeled{lc.calib.YAW, InputNumber{config.manualCalib.angleOffset[1]}})
        .Add(Labeled{lc.calib.ROLL, InputNumber{config.manualCalib.angleOffset[2]}})
        .Add(Labeled{lc.calib.SCALE, InputNumber{config.manualCalib.scale}})
        .Add(CheckBoxButton{lc.CAMERA_MULTICAM_CALIB, [this](auto& evt)
            {
                tracker->multicamAutocalib = evt.IsChecked();
            }})
        .Add(CheckBoxButton{lc.CAMERA_LOCK_HEIGHT, [this](auto& evt)
            {
                tracker->lockHeightCalib = evt.IsChecked();
            }});

    manualCalibForm->SetSizerVisible(false);
}

void GUI::MainFrame::CreateParamsPage(RefPtr<wxNotebook> pages)
{
    using namespace Form;

    auto panel = NewWindow<wxPanel>(pages);
    pages->AddPage(panel, lc.TAB_PARAMS);

    auto boxSizer = NewSizer<wxBoxSizer>(panel, wxVERTICAL);
    params = FormBuilder{panel, boxSizer};

    static constexpr std::array<U8StringView, 4> markerLibraries =
    { "AprilTag Standard", "AprilTag Circular", "Aruco4x4", "AprilTag Color" };

    static constexpr std::array<U8StringView, 4> camRotOptions =
    { "0", "90", "180", "270" };

    static constexpr std::array<int, 4> camRotCodes =
    { -1, cv::ROTATE_90_CLOCKWISE, cv::ROTATE_180, cv::ROTATE_90_COUNTERCLOCKWISE };

    static constexpr std::array<U8StringView, 6> quadDecimateOptions =
    { "1", "1.5", "2", "3", "4", "5" };

    static constexpr std::array<double, 6> quadDecimateValues =
    { 1, 1.5, 2, 3, 4, 5 };

    params.Border(wxALL, 5)
        .PushSizer<wxFlexGridSizer>(4, wxSize(10, 10))
        .Add(Labeled{lc.PARAMS_LANGUAGE,
            Choice{config.langCode, Localization::LANG_NAME_MAP, Localization::LANG_CODE_MAP}})
        .Add(Labeled{lc.WINDOW_TITLE, lc.WINDOW_TITLE_TOOLTIP,
            InputText{config.windowTitle}})
        .PopSizer()
        .PushStaticBoxSizer(lc.PARAMS_CAMERA)
        .PushSizer<wxFlexGridSizer>(4, wxSize(10, 10))
        .Add(Labeled{lc.PARAMS_CAMERA_NAME_ID, lc.PARAMS_CAMERA_TOOLTIP_ID,
            InputText{config.cameraAddr}})
        .Add(Labeled{lc.PARAMS_CAMERA_NAME_API, CreateCVCaptureAPIToolTip(lc),
            InputText{config.cameraApiPreference}})
        .Add(Labeled{ lc.PARAMS_CAMERA_NAME_ROT_CLOCKWISE, lc.PARAMS_CAMERA_TOOLTIP_ROT_CLOCKWISE,
            Choice{config.rotateCl, camRotOptions, camRotCodes} })
        .Add(Labeled{lc.PARAMS_CAMERA_NAME_MIRROR, lc.PARAMS_CAMERA_TOOLTIP_MIRROR,
            CheckBox{config.mirrorCam}})
        .Add(Labeled{lc.PARAMS_CAMERA_NAME_WIDTH, lc.PARAMS_CAMERA_TOOLTIP_WIDTH,
            InputText{config.camWidth}})
        .Add(Labeled{lc.PARAMS_CAMERA_NAME_HEIGHT, lc.PARAMS_CAMERA_TOOLTIP_HEIGHT,
            InputText{config.camHeight}})
        .Add(Labeled{lc.PARAMS_CAMERA_NAME_FPS, lc.PARAMS_CAMERA_TOOLTIP_FPS,
            InputText{config.camFps}})
        .Add(Labeled{lc.PARAMS_CAMERA_NAME_SETTINGS, lc.PARAMS_CAMERA_TOOLTIP_SETTINGS,
            CheckBox{config.cameraSettings}})
        .PopSizer()
        .PushStaticBoxSizer("LIGHTING")
        .PushSizer<wxFlexGridSizer>(4, wxSize(10, 10));

    auto extraCheck = params.AddGet(Labeled{lc.PARAMS_CAMERA_NAME_3_OPTIONS,
        lc.PARAMS_CAMERA_TOOLTIP_3_OPTIONS,
        CheckBox{config.settingsParameters}});

    auto extraOpts = params.SubForm();
    extraOpts
        ->Add(Labeled{lc.PARAMS_CAMERA_NAME_AUTOEXP, lc.PARAMS_CAMERA_TOOLTIP_AUTOEXP,
            InputText{config.cameraAutoexposure}})
        .Add(Labeled{lc.PARAMS_CAMERA_NAME_EXP, lc.PARAMS_CAMERA_TOOLTIP_EXP,
            InputText{config.cameraExposure}})
        .Add(Labeled{lc.PARAMS_CAMERA_NAME_GAIN, lc.PARAMS_CAMERA_TOOLTIP_GAIN,
            InputText{config.cameraGain}});

    extraOpts->SetEnabled(config.settingsParameters);

    extraCheck->GetElem().Bind(wxEVT_CHECKBOX,
        [=](wxCommandEvent& evt)
        {
            extraOpts->SetEnabled(evt.IsChecked());
        });

    params.PopSizer()
        .PopSizer()
        .PopSizer()
        .PushStaticBoxSizer(lc.PARAMS_TRACKER)
        .PushSizer<wxFlexGridSizer>(4, wxSize(10, 10))
        .Add(Labeled{lc.PARAMS_TRACKER_NAME_NUM_TRACKERS, lc.PARAMS_TRACKER_TOOLTIP_NUM_TRACKERS,
            InputText{config.trackerNum}})
        .Add(Labeled{lc.PARAMS_TRACKER_NAME_MARKER_SIZE, lc.PARAMS_TRACKER_TOOLTIP_MARKER_SIZE,
            InputText{config.markerSize}})
        .Add(Labeled{lc.PARAMS_TRACKER_NAME_QUAD_DECIMATE, lc.PARAMS_TRACKER_TOOLTIP_QUAD_DECIMATE,
            Choice{config.quadDecimate, quadDecimateOptions, quadDecimateValues}})
        .Add(Labeled{lc.PARAMS_TRACKER_NAME_SEARCH_WINDOW, lc.PARAMS_TRACKER_TOOLTIP_SEARCH_WINDOW,
            InputText{config.searchWindow}})
        .Add(Labeled{lc.PARAMS_TRACKER_NAME_MARKER_LIBRARY, lc.PARAMS_TRACKER_TOOLTIP_MARKER_LIBRARY,
            Choice{config.markerLibrary, markerLibraries}})
        .Add(Labeled{lc.PARAMS_TRACKER_NAME_USE_CENTERS, lc.PARAMS_TRACKER_TOOLTIP_USE_CENTERS,
            CheckBox{config.trackerCalibCenters}})
        .Add(Labeled{lc.PARAMS_TRACKER_NAME_IGNORE_0, lc.PARAMS_TRACKER_TOOLTIP_IGNORE_0,
            CheckBox{config.ignoreTracker0}})
        .PopSizer()
        .PopSizer()
        .PushStaticBoxSizer(lc.PARAMS_SMOOTHING)
        .PushSizer<wxFlexGridSizer>(4, wxSize(10, 10))
        .Add(Labeled{lc.PARAMS_SMOOTHING_NAME_WINDOW, lc.PARAMS_SMOOTHING_TOOLTIP_WINDOW,
            InputText{config.smoothingFactor}})
        .Add(Labeled{lc.PARAMS_SMOOTHING_NAME_ADDITIONAL, lc.PARAMS_SMOOTHING_TOOLTIP_ADDITIONAL,
            InputText{config.additionalSmoothing}})
        .Add(Labeled{lc.PARAMS_SMOOTHING_NAME_DEPTH, lc.PARAMS_SMOOTHING_TOOLTIP_DEPTH,
            InputText{config.depthSmoothing}})
        .Add(Labeled{lc.PARAMS_SMOOTHING_NAME_CAM_LATENCY, lc.PARAMS_SMOOTHING_TOOLTIP_CAM_LATENCY,
            InputText{config.camLatency}})
        .PopSizer()
        .PopSizer()
        .Add(Button{lc.PARAMS_SAVE, [&](auto&)
            {
                SaveParams();
            }});
}

void GUI::MainFrame::CreateLicensePage(RefPtr<wxNotebook> pages)
{
    auto panel = NewWindow<wxPanel>(pages);
    pages->AddPage(panel, lc.TAB_LICENSE);

    auto boxSizer = NewSizer<wxBoxSizer>(panel, wxHORIZONTAL);

    auto nb = NewWindow<wxNotebook>(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
    boxSizer->Add(nb, wxSizerFlags(1).Expand());

    auto ourLicense = NewWindow<wxTextCtrl>(nb, wxID_ANY, std::string(ATT_LICENSE), wxDefaultPosition,
        wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    nb->AddPage(ourLicense, "AprilTagTrackers", true);
    auto aprilLicense = NewWindow<wxTextCtrl>(nb, wxID_ANY, std::string(APRILTAG_LICENSE),
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    nb->AddPage(aprilLicense, "AprilTag");
    auto cvLicense = NewWindow<wxTextCtrl>(nb, wxID_ANY, std::string(OPENCV_LICENSE), wxDefaultPosition,
        wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    nb->AddPage(cvLicense, "OpenCV");
}

/// Query OpenCV to find available camera api indices
U8String GUI::MainFrame::CreateCVCaptureAPIToolTip(const Localization& lc)
{
    std::ostringstream cameraTooltipApi;
    cameraTooltipApi << lc.PARAMS_CAMERA_TOOLTIP_API_1;
    for (const auto& backend : cv::videoio_registry::getCameraBackends())
    {
        cameraTooltipApi << "\n"
                         << static_cast<int>(backend) << ": "
                         << cv::videoio_registry::getBackendName(backend);
    }
#ifdef ATT_ENABLE_PS3EYE
    cameraTooltipApi << "\n9100: PS3EYE";
#endif
    cameraTooltipApi << "\n\n"
                     << lc.PARAMS_CAMERA_TOOLTIP_API_2;
    for (const auto& backend : cv::videoio_registry::getStreamBackends())
    {
        cameraTooltipApi << "\n"
                         << static_cast<int>(backend) << ": "
                         << cv::videoio_registry::getBackendName(backend);
    }
    return cameraTooltipApi.str();
}
