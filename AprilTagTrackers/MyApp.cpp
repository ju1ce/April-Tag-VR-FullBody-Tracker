#include "Connection.h"
#include "GUI.h"
#include "Helpers.h"
#include "MyApp.h"
#include "Parameter.h"
#include "Tracker.h"
#include "ConfigStorage.h"

wxIMPLEMENT_APP(MyApp);

int MyApp::OnExit()
{
    tracker->cameraRunning = false;
    tracker->mainThreadRunning = false;
    sleep_millis(2000);

    delete params;
    delete conn;
    delete tracker;

    return 0;
}

bool MyApp::OnInit()
{


    conn = new Connection(params);
    tracker = new Tracker(params, conn, this);

    gui = new GUI(params->language.APP_TITLE,params,conn);

    conn->gui = gui; // juice told me to write this, dont blame me

    ArucoConfigStorage ac;
    ac.Save("aruco.yaml");

    gui->Show(true);

    gui->posHbox->Show(false);
    gui->rotHbox->Show(false);

    tracker->gui = gui;

    Connect(GUI::CAMERA_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCamera));
    Connect(GUI::CAMERA_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraPreview));
    Connect(GUI::CAMERA_CALIB_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraCalib));
    Connect(GUI::CAMERA_CALIB_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraCalibPreview));
    //Connect(GUI::TIME_PROFILE_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedTimeProfile));
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
    tracker->StartCamera(params->cameraAddr, params->cameraApiPreference);
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
        conn->disableOpenVrApi = true;
    }
    else
    {
        tracker->disableOpenVrApi = false;
        conn->disableOpenVrApi = false;
    }
}

void MyApp::ButtonPressedSpaceCalib(wxCommandEvent& event)
{
    if (event.GetId() == GUI::SPACE_CALIB_CHECKBOX)
    {
        //deprecated
        if (event.IsChecked())
        {
            tracker->recalibrate = true;
            gui->posHbox->Show(true);
            gui->rotHbox->Show(false);
            gui->cb3->SetValue(false);
            tracker->manualRecalibrate = false;

            gui->manualCalibX->SetValue(params->calibOffsetX);
            gui->manualCalibY->SetValue(params->calibOffsetY);
            gui->manualCalibZ->SetValue(params->calibOffsetZ);

        }
        else
        {
            params->wrotation = tracker->wrotation;
            params->wtranslation = tracker->wtranslation;
            params->calibOffsetX = gui->manualCalibX->value;
            params->calibOffsetY = gui->manualCalibY->value;
            params->calibOffsetZ = gui->manualCalibZ->value;
            params->Save();
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
            //gui->cb2->SetValue(false);
            tracker->recalibrate = false;

            gui->manualCalibX->SetValue(params->calibOffsetX);
            gui->manualCalibY->SetValue(params->calibOffsetY);
            gui->manualCalibZ->SetValue(params->calibOffsetZ);
            gui->manualCalibA->SetValue(params->calibOffsetA);
            gui->manualCalibB->SetValue(params->calibOffsetB);
            gui->manualCalibC->SetValue(params->calibOffsetC);

            tracker->calibScale = params->calibScale;

            tracker->manualRecalibrate = true;
        }
        else
        {
            params->wrotation = tracker->wrotation;
            params->wtranslation = tracker->wtranslation;
            params->calibOffsetX = gui->manualCalibX->value;
            params->calibOffsetY = gui->manualCalibY->value;
            params->calibOffsetZ = gui->manualCalibZ->value;
            params->calibOffsetA = gui->manualCalibA->value;
            params->calibOffsetB = gui->manualCalibB->value;
            params->calibOffsetC = gui->manualCalibC->value;
            params->calibScale = tracker->calibScale;

            params->Save();
            tracker->manualRecalibrate = false;
            gui->posHbox->Show(false);
            gui->rotHbox->Show(false);
        }
    }
}
