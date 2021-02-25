#include "Connection.h"
#include "GUI.h"
#include "Helpers.h"
#include "MyApp.h"
#include "Parameters.h"
#include "Tracker.h"

wxIMPLEMENT_APP(MyApp);

int MyApp::OnExit()
{
    tracker->cameraRunning = false;
    tracker->mainThreadRunning = false;
    tracker.reset();
    params.reset();
    conn.reset();
    gui.reset();
    Sleep(2000);
    return 0;
}

bool MyApp::OnInit()
{
    params = std::make_shared<Parameters>();
    conn = std::make_shared<Connection>(params);
    tracker = std::make_shared<Tracker>(params, conn);

    gui = std::make_shared<GUI>(wxT("AprilTag Trackers"), params);
    gui->Show(true);

    gui->posHbox->Show(false);
    gui->rotHbox->Show(false);

    tracker->gui = gui;

    Connect(GUI::CAMERA_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCamera));
    Connect(GUI::CAMERA_CALIB_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraCalib));
    Connect(GUI::CAMERA_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraPreview));
    Connect(GUI::CONNECT_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedConnect));
    Connect(GUI::TRACKER_CALIB_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedTrackerCalib));
    Connect(GUI::START_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedStart));
    Connect(GUI::SPACE_CALIB_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedSpaceCalib));
    Connect(GUI::MANUAL_CALIB_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedSpaceCalib));

    return true;
}

void MyApp::ButtonPressedCamera(wxCommandEvent& event)
{
    tracker->StartCamera(params->cameraAddr);
}

void MyApp::ButtonPressedCameraPreview(wxCommandEvent& event)
{
    if (event.IsChecked())
        tracker->previewCamera = true;
    else
        tracker->previewCamera = false;
}

void MyApp::ButtonPressedCameraCalib(wxCommandEvent& event)
{
    tracker->StartCameraCalib();
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
            tracker->manualRecalibrate = true;
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
            params->Save();
            tracker->manualRecalibrate = false;
            gui->posHbox->Show(false);
            gui->rotHbox->Show(false);
        }
    }
}
