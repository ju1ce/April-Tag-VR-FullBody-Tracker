#pragma once

#include "Config.h"
#include "Connection.h"
#include "Localization.h"

// TODO: Don't include wx headers in our headers, except wxString
#pragma warning(push)
#pragma warning(disable : 4996)
#include <wx/notebook.h>
#include <wx/timer.h>
#include <wx/wx.h>

#pragma warning(pop)

#include <mutex>
#include <thread>
#include <type_traits>

class ValueInput : public wxPanel
{
public:
    ValueInput(wxPanel* parent, const wxString& nm, double val);
    double value;
    void SetValue(double val);

private:
    wxTextCtrl* input = nullptr;

    void ButtonPressed(wxCommandEvent&);
    void MouseScroll(wxMouseEvent&);
};

/// OpenCV high level gui window
class PreviewWindow
{
public:
    static PreviewWindow Camera;
    static PreviewWindow Out;

    /// Closes the high gui window.
    void Close();
    /// Checks if opencv internally has a window with windowName
    bool IsVisible() const;
    /// Set the window name/titlebar
    void SetWindowTitle(const std::string& title);
    /// Locks imageMutex, and makes a deep copy of newImage.
    void CloneImage(const cv::Mat& newImage);
    /// Locks imageMutex, cv::swap image with newImage.
    void SwapImage(cv::Mat& newImage);

    bool operator==(const PreviewWindow& other) const { return windowName == other.windowName; }
    bool operator!=(const PreviewWindow& other) const { return !(*this == other); }

private:
    // Make constructor private, as instances should only be handled by this classes static fields
    PreviewWindow(std::string _windowName);
    // static destruction at application exit, so high gui windows will be cleaned up
    // OpenCV docs say it's mostly unecessary to call cv::destroyWindow() at application exit
    ~PreviewWindow();

    /// Handles the update loop, drawing and events for opencv high gui
    /// Only one instance needed, static field Timer
    class UpdateTimer : public wxTimer
    {
    public:
        UpdateTimer() : wxTimer()
        {
            constexpr int fps = 30;
            constexpr int millis = 1000 / fps;
            // Start the timer, it will be running for duration of application
            wxTimer::Start(millis);
        }

        void AddWindow(const PreviewWindow& window);
        void RemoveWindow(const PreviewWindow& window);

    private:
        /// Called on each tick of the timer, from gui thread
        void Notify() override;
        /// Windows to be updated
        std::vector<std::reference_wrapper<const PreviewWindow>> windowList;
    };
    /// OpenCV update loop singleton
    static UpdateTimer Timer;

    /// Locks imageMutex, Applies the image to the window.
    void UpdateWindow() const;
    /// Name of the window, aswell as the only way to reference it.
    std::string windowName;
    /// Matrix representing an image.
    cv::Mat image;
    /// Only show frame if image was updated
    bool newImageReady = false;
    /// Locks image and newImageReady
    mutable std::mutex imageMutex;
};

class GUI : protected wxFrame
{
public:
    GUI(const wxString& title, Connection* conn, UserConfig& userConfig, const Localization& lcl);

    /// Queue a lambda to be run on the GUI thread, after other events have been processed.
    /// On MSW the event will not be processed as soon as possible if a Modal is shown.
    /// lambda capture references should point to the same memory, copies will be copied again.
    /// From wx docs:
    ///     these methods schedule the given method pointer
    ///     for a later call (during the next idle event loop iteration)
    template <typename T>
    void QueueOnGUIThread(const T& func)
    {
        CallAfter(func);
    }

    // TODO: Ok to call from within another CallAfter?
    /// Queue a popup to be shown by the GUI thread.
    void QueuePopup(const wxString& content, const wxString& caption, long style);

    void ShowErrorPopup(const wxString& content) { QueuePopup(content, wxT("Error"), wxOK | wxICON_ERROR); }
    void ShowWarningPopup(const wxString& content) { QueuePopup(content, wxT("Warning"), wxOK | wxICON_WARNING); }
    void ShowInfoPopup(const wxString& content) { QueuePopup(content, wxT("Info"), wxOK | wxICON_INFORMATION); }

    ValueInput* manualCalibX;
    ValueInput* manualCalibY;
    ValueInput* manualCalibZ;
    ValueInput* manualCalibA;
    ValueInput* manualCalibB;
    ValueInput* manualCalibC;

    wxBoxSizer* posHbox;
    wxBoxSizer* rotHbox;

    wxCheckBox* cb2;
    wxCheckBox* cb3;
    wxCheckBox* cb4;
    wxCheckBox* cb5;

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
    static const int DISABLE_OUT_CHECKBOX = 12;
    static const int DISABLE_OPENVR_API_CHECKBOX = 13;
};

class LicensePage : public wxPanel
{
public:
    explicit LicensePage(wxNotebook* parent);
};

class CameraPage : public wxPanel
{
public:
    CameraPage(wxNotebook* parent, GUI* parentGUI, const Localization& lcl);
};

class ParamsPage : public wxPanel
{
public:
    ParamsPage(wxNotebook* parent, Connection* conn, UserConfig& user_config, const Localization& lcl);

private:
    const int SAVE_BUTTON = 2;
    const int HELP_BUTTON = 10;

    UserConfig& user_config;
    const Localization& lc;

    Connection* connection;
    wxTextCtrl* windowTitleField;
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
    wxChoice* languageField;

    void SaveParams(wxCommandEvent&);
};
