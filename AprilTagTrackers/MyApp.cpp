#include "MyApp.h"

#include "Connection.h"
#include "Debug.h"
#include "GUI.h"
#include "Tracker.h"

#ifdef ATT_OVERRIDE_ERROR_HANDLERS
#include <opencv2/core/utils/logger.hpp>

#include <exception>
#include <stdexcept>
#endif

#ifdef ATT_ENABLE_OUTPUT_LOG_FILE
#include <fstream>
#include <iostream>
#endif

wxIMPLEMENT_APP(MyApp);

int MyApp::OnExit()
{
    tracker->cameraRunning = false;
    tracker->mainThreadRunning = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    delete conn;
    delete tracker;

    return 0;
}

bool MyApp::OnInit()
{
    user_config.Load();
    calib_config.Load();
    aruco_config.Load();
    lc.LoadLang(user_config.langCode);

    conn = new Connection(user_config, lc);
    tracker = new Tracker(this, conn, user_config, calib_config, lc, aruco_config);

    gui = new GUI(user_config.windowTitle.empty() ? lc.APP_TITLE : user_config.windowTitle, conn, user_config, lc);

    conn->gui = gui; // juice told me to write this, dont blame me

    gui->posHbox->Show(false);
    gui->rotHbox->Show(false);

    tracker->gui = gui;

    Connect(GUI::CAMERA_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCamera));
    Connect(GUI::CAMERA_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraPreview));
    Connect(GUI::CAMERA_CALIB_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraCalib));
    Connect(GUI::CAMERA_CALIB_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraCalibPreview));
    // Connect(GUI::TIME_PROFILE_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedTimeProfile));
    Connect(GUI::CONNECT_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedConnect));
    Connect(GUI::TRACKER_CALIB_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedTrackerCalib));
    Connect(GUI::START_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedStart));
    Connect(GUI::SPACE_CALIB_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedSpaceCalib));
    Connect(GUI::MANUAL_CALIB_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedSpaceCalib));
    Connect(GUI::MULTICAM_AUTOCALIB_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedMulticamAutocalib));
    Connect(GUI::LOCK_HEIGHT_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedLockHeight));
    Connect(GUI::DISABLE_OUT_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedDisableOut));
    Connect(GUI::DISABLE_OPENVR_API_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedDisableOpenVrApi));

    return true;
}

void MyApp::ButtonPressedCamera(wxCommandEvent& event)
{
    tracker->StartCamera(user_config.cameraAddr, user_config.cameraApiPreference);
}

void MyApp::ButtonPressedCameraPreview(wxCommandEvent& event)
{
    if (event.IsChecked())
        gui->previewWindow.Show();
    else
        gui->previewWindow.Hide();
}

void MyApp::ButtonPressedCameraCalib(wxCommandEvent& event)
{
    tracker->StartCameraCalib();
}

void MyApp::ButtonPressedCameraCalibPreview(wxCommandEvent& event)
{
    tracker->previewCameraCalibration = event.IsChecked();
}

void MyApp::ButtonPressedTimeProfile(wxCommandEvent& event)
{
    tracker->showTimeProfile = event.IsChecked();
}

void MyApp::ButtonPressedConnect(wxCommandEvent& event)
{
    conn->StartConnection();
}

void MyApp::ButtonPressedTrackerCalib(wxCommandEvent& event)
{
    tracker->StartTrackerCalib();
}

void MyApp::ButtonPressedStart(wxCommandEvent& event)
{
    tracker->Start();
}

void MyApp::ButtonPressedMulticamAutocalib(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        tracker->multicamAutocalib = true;
    }
    else
    {
        tracker->multicamAutocalib = false;
    }
}

void MyApp::ButtonPressedLockHeight(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        tracker->lockHeightCalib = true;
    }
    else
    {
        tracker->lockHeightCalib = false;
    }
}

void MyApp::ButtonPressedDisableOut(wxCommandEvent& event)
{
    if (event.IsChecked())
        gui->outWindow.Hide();
    else
        gui->outWindow.Show();
}

void MyApp::ButtonPressedDisableOpenVrApi(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        user_config.disableOpenVrApi = true;
    }
    else
    {
        user_config.disableOpenVrApi = false;
    }
    user_config.Save();
}

void MyApp::ButtonPressedSpaceCalib(wxCommandEvent& event)
{
    if (event.GetId() == GUI::SPACE_CALIB_CHECKBOX)
    {
        // deprecated
        if (event.IsChecked())
        {
            tracker->recalibrate = true;
            gui->posHbox->Show(true);
            gui->rotHbox->Show(false);
            gui->cb3->SetValue(false);
            tracker->manualRecalibrate = false;

            gui->manualCalibX->SetValue(user_config.calibOffsetX);
            gui->manualCalibY->SetValue(user_config.calibOffsetY);
            gui->manualCalibZ->SetValue(user_config.calibOffsetZ);
        }
        else
        {
            calib_config.wrotation = tracker->wrotation;
            calib_config.wtranslation = tracker->wtranslation;
            user_config.calibOffsetX = gui->manualCalibX->value;
            user_config.calibOffsetY = gui->manualCalibY->value;
            user_config.calibOffsetZ = gui->manualCalibZ->value;
            user_config.Save();
            tracker->recalibrate = false;
            gui->posHbox->Show(false);
            gui->rotHbox->Show(false);
        }
    }
    if (event.GetId() == GUI::MANUAL_CALIB_CHECKBOX)
    {
        if (event.IsChecked())
        {
            gui->posHbox->Show(true);
            gui->rotHbox->Show(true);
            // gui->cb2->SetValue(false);
            tracker->recalibrate = false;

            gui->manualCalibX->SetValue(user_config.calibOffsetX);
            gui->manualCalibY->SetValue(user_config.calibOffsetY);
            gui->manualCalibZ->SetValue(user_config.calibOffsetZ);
            gui->manualCalibA->SetValue(user_config.calibOffsetA);
            gui->manualCalibB->SetValue(user_config.calibOffsetB);
            gui->manualCalibC->SetValue(user_config.calibOffsetC);

            tracker->calibScale = user_config.calibScale;

            tracker->manualRecalibrate = true;
        }
        else
        {
            calib_config.wrotation = tracker->wrotation;
            calib_config.wtranslation = tracker->wtranslation;
            user_config.calibOffsetX = gui->manualCalibX->value;
            user_config.calibOffsetY = gui->manualCalibY->value;
            user_config.calibOffsetZ = gui->manualCalibZ->value;
            user_config.calibOffsetA = gui->manualCalibA->value;
            user_config.calibOffsetB = gui->manualCalibB->value;
            user_config.calibOffsetC = gui->manualCalibC->value;
            user_config.calibScale = tracker->calibScale;

            user_config.Save();
            tracker->manualRecalibrate = false;
            gui->posHbox->Show(false);
            gui->rotHbox->Show(false);
        }
    }
}

#ifdef ATT_ENABLE_OUTPUT_LOG_FILE

static std::ofstream outputLogFileWriter{"output.log"};

static const bool consoleOutputRedirected = ([]()
    {
        std::cout.rdbuf(outputLogFileWriter.rdbuf());
        std::cerr.rdbuf(outputLogFileWriter.rdbuf());
        return true; //
    })();

#endif

#ifdef ATT_OVERRIDE_ERROR_HANDLERS

// Expand in place to maintain stack frames
#define HANDLE_UNHANDLED_EXCEPTION(a_msgContext)          \
    const auto ePtr = std::current_exception();           \
    try                                                   \
    {                                                     \
        if (ePtr) std::rethrow_exception(ePtr);           \
    }                                                     \
    catch (const std::exception& e)                       \
    {                                                     \
        ATFATAL(a_msgContext << ": " << e.what());        \
    }                                                     \
    catch (...)                                           \
    {                                                     \
        ATFATAL(a_msgContext << ": malformed exception"); \
    }                                                     \
    ATFATAL(a_msgContext << ": unknown exception");

void MyApp::OnFatalException()
{
    HANDLE_UNHANDLED_EXCEPTION("wxApp::OnFatalException");
}
void MyApp::OnUnhandledException()
{
    HANDLE_UNHANDLED_EXCEPTION("wxApp::OnUnhandledException");
}
bool MyApp::OnExceptionInMainLoop()
{
    HANDLE_UNHANDLED_EXCEPTION("wxApp::OnExceptionInMainLoop");
    return true; // suppress warning
}

// cv::ErrorCallback
static int OpenCVErrorHandler(int status, const char* funcName, const char* errMsg, const char* fileName, int line, void*)
{
#if ATT_LOG_LEVEL >= 1
    Debug::PreLog(fileName, line);
    std::cerr << "OpenCV Error: " << errMsg << std::endl
              << "    in: " << funcName << std::endl;
#endif
    Debug::abort();
    return status;
}

// wxAssertHandler_t
static void wxWidgetsAssertHandler(const wxString& file, int line, const wxString& func, const wxString& cond, const wxString& msg)
{
#if ATT_LOG_LEVEL >= 1
    Debug::PreLog(file.c_str().AsChar(), line);
    std::cerr << "wxWidgets Error: " << msg << std::endl
              << "    Assertion failure:  ( " << cond << " )  in: " << func << std::endl;
#endif
#ifdef ATT_ENABLE_ASSERT
    Debug::abort();
#endif
}

static const bool errorHandlersRedirected = ([]()
    {
        cv::redirectError(&OpenCVErrorHandler);
        cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
        wxSetAssertHandler(&wxWidgetsAssertHandler);
        return true; //
    })();
#endif
