#include "MyApp.h"
#include "Connection.h"
#include "GUI.h"
#include "Tracker.h"

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

    gui = new GUI(lc.APP_TITLE, conn, user_config, lc);

    conn->gui = gui; // juice told me to write this, dont blame me

    gui->Show(true);

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
    tracker->previewCamera = event.IsChecked();
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
    {
        tracker->disableOut = true;
    }
    else
    {
        tracker->disableOut = false;
    }
}

void MyApp::ButtonPressedDisableOpenVrApi(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        tracker->disableOpenVrApi = true;
        user_config.disableOpenVrApi = true;
    }
    else
    {
        tracker->disableOpenVrApi = false;
        user_config.disableOpenVrApi = false;
    }
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
