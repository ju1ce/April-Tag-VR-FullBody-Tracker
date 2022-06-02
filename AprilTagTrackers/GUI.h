#pragma once

#include "Config.h"
#include "Localization.h"
#include "RefPtr.h"

#include <opencv2/core.hpp>

#include <functional>

class GUI;

// Interface for gui to control the tracker
class ITrackerControl
{
public:
    friend class GUI;

    virtual ~ITrackerControl() = default;

    virtual void StartCamera() = 0;
    virtual void StartCameraCalib() = 0;
    virtual void StartTrackerCalib() = 0;
    virtual void StartConnection() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void UpdateConfig() = 0;

    /// bools set by GUI that could and probably should be functions
    bool previewCameraCalibration = false;
    bool manualRecalibrate = false;
    bool multicamAutocalib = false;
    bool lockHeightCalib = false;

protected:
    /// Not set during tracker contstruction
    RefPtr<GUI> gui;

private:
    /// Called from GUI's ctor
    void SetGUI(RefPtr<GUI> _gui) { gui = _gui; }
};

enum class PopupStyle
{
    Error,
    Warning,
    Info
};

enum class StatusItem
{
    Camera,
    Driver,
    Tracker,
    COUNT
};

enum class PreviewId
{
    Main,
    Camera
};

class PreviewControl
{
public:
    PreviewControl(RefPtr<GUI> _gui, PreviewId _id);
    ~PreviewControl();
    void Update(const cv::Mat& image);
    bool IsVisible();

private:
    RefPtr<GUI> gui;
    PreviewId id;
};

/// Thread safe GUI interface, anything that needs to happen
/// on the main thread will get wrapped in a CallAfter here.
class GUI
{
public:
    GUI(RefPtr<ITrackerControl> _tracker, const Localization& _lc, UserConfig& _config);
    ~GUI();

    void ShowPrompt(U8String msg, std::function<void(bool)> onClose);
    void ShowPopup(U8String msg, PopupStyle style);

    void SetStatus(bool status, StatusItem item);

    PreviewControl CreatePreviewControl(PreviewId id = PreviewId::Main);
    void SetPreviewVisible(bool visible = true, PreviewId id = PreviewId::Main, bool userCanDestroy = true);
    void UpdatePreview(const cv::Mat& image, PreviewId id = PreviewId::Main);
    bool IsPreviewVisible(PreviewId id = PreviewId::Main);

    /// Get the manual calibration currently shown in the UI
    ManualCalib::Real GetManualCalib();
    /// Set the manual calib currently shown in the UI
    void SetManualCalib(const ManualCalib::Real& calib);
    /// Set if the manual calib window is visible.
    void SetManualCalibVisible(bool visible = true);
    /// Save manual calib to user config
    void SaveManualCalib();

    /// Defined in GUI/MainFrame.h, but used as opaque ptr, not to be included in headers.
    class MainFrame;

private:
    // wxWidgets owns and will free in close window event
    RefPtr<MainFrame> impl;
};
