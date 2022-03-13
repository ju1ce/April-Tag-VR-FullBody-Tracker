#include "Connection.h"

#include "GUI.h"
#include "Util.h"
#include <thread>

Connection::Connection(const UserConfig& user_config, const Localization& lcl)
    : user_config(user_config), lcl(lcl)
{
// TODO: Pass the IPC client* in as an argument
#if OS_WIN
    auto* namedPipe = new IPC::WindowsNamedPipe("ApriltagPipeIn");
    bridge_driver.reset(dynamic_cast<IPC::IClient*>(namedPipe));
#elif OS_LINUX
    auto namedPipe = new IPC::UNIXSocket("ApriltagPipeIn");
    bridge_driver.reset(dynamic_cast<IPC::IClient*>(namedPipe));
#endif
}

void Connection::StartConnection()
{
    if (status == WAITING)
    {
        gui->CallAfter([]()
                       {
                       wxMessageDialog dial(NULL,
                           wxT("Already waiting for a connection"), wxT("Error"), wxOK | wxICON_ERROR);
                       dial.ShowModal(); });
        return;
    }
    if (status == CONNECTED)
    {
        gui->CallAfter([this]()
                       {
                       wxMessageDialog dial(NULL,
                           lcl.CONNECT_ALREADYCONNECTED, wxT("Question"),
                           wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
                       dial.ShowModal(); });
        sleep_millis(1000);
        // Sleep(1000);
        status = DISCONNECTED;
    }
    std::thread connectThread(&Connection::Connect, this);
    connectThread.detach();
}

void Connection::Connect()
{
    // generate vector of tracker connection struct, connecting board ids to apropriate driver ids. In future, this should be done manualy in the gui
    connectedTrackers.clear();

    if (user_config.ignoreTracker0 && user_config.trackerNum == 3)
    {
        for (int i = 0; i < user_config.trackerNum - 1; i++)
        {
            TrackerConnection temp;
            temp.TrackerId = i + 1;
            temp.DriverId = i;
            temp.Name = "ApriltagTracker" + std::to_string(i + 1);
            connectedTrackers.push_back(temp);
        }
        connectedTrackers[0].Role = "TrackerRole_LeftFoot";
        connectedTrackers[1].Role = "TrackerRole_RightFoot";
    }
    else if (user_config.ignoreTracker0)
    {
        for (int i = 0; i < user_config.trackerNum - 1; i++)
        {
            TrackerConnection temp;
            temp.TrackerId = i + 1;
            temp.DriverId = i;
            temp.Name = "ApriltagTracker" + std::to_string(i + 1);
            temp.Role = "TrackerRole_Waist";
            connectedTrackers.push_back(temp);
        }
    }
    else if (user_config.trackerNum == 3)
    {
        for (int i = 0; i < user_config.trackerNum; i++)
        {
            TrackerConnection temp;
            temp.TrackerId = i;
            temp.DriverId = i;
            temp.Name = "ApriltagTracker" + std::to_string(i);
            connectedTrackers.push_back(temp);
        }
        connectedTrackers[0].Role = "TrackerRole_Waist";
        connectedTrackers[1].Role = "TrackerRole_LeftFoot";
        connectedTrackers[2].Role = "TrackerRole_RightFoot";
    }
    else if (user_config.trackerNum == 2)
    {
        for (int i = 0; i < user_config.trackerNum; i++)
        {
            TrackerConnection temp;
            temp.TrackerId = i;
            temp.DriverId = i;
            temp.Name = "ApriltagTracker" + std::to_string(i);
            connectedTrackers.push_back(temp);
        }
        connectedTrackers[0].Role = "TrackerRole_LeftFoot";
        connectedTrackers[1].Role = "TrackerRole_RightFoot";
    }
    else
    {
        for (int i = 0; i < user_config.trackerNum; i++)
        {
            TrackerConnection temp;
            temp.TrackerId = i;
            temp.DriverId = i;
            temp.Name = "ApriltagTracker" + std::to_string(i + 1);
            temp.Role = "TrackerRole_Waist";
            connectedTrackers.push_back(temp);
        }
    }

    if (!user_config.disableOpenVrApi)
    {
        // connect to steamvr as a client in order to get buttons.
        vr::EVRInitError error;
        openvr_handle = VR_Init(&error, vr::VRApplication_Overlay);

        if (error != vr::VRInitError_None)
        {
            wxString e = lcl.CONNECT_CLIENT_ERROR;
            e += vr::VR_GetVRInitErrorAsEnglishDescription(error);
            gui->CallAfter([e]()
                           {
                           wxMessageDialog dial(NULL,
                               e, wxT("Error"), wxOK | wxICON_ERROR);
                           dial.ShowModal(); });
            status = DISCONNECTED;
            return;
        }
    }

    /*
    vr::HmdMatrix34_t testZeroToStanding = openvr_handle->GetRawZeroPoseToStandingAbsoluteTrackingPose();

    std::string e = "Zero pose to standing: \n ";
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            e += std::to_string(testZeroToStanding.m[i][j]) + ", ";
        }
        e += "\n";
    }
    wxMessageDialog dial(NULL,
        e, wxT("Error"), wxOK | wxICON_ERROR);
    dial.ShowModal();

    vr::HmdMatrix34_t testStandingToSeated = openvr_handle->GetSeatedZeroPoseToStandingAbsoluteTrackingPose();

    e = "Seated pose to standing: \n ";
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            e += std::to_string(testStandingToSeated.m[i][j]) + ", ";
        }
        e += "\n";
    }
    wxMessageDialog dial2(NULL,
        e, wxT("Error"), wxOK | wxICON_ERROR);
    dial2.ShowModal();

    */
    std::string fullpath;

    if (!user_config.disableOpenVrApi)
    {
        if (!get_full_path("att_actions.json", fullpath))
        {
            perror("Could not find bindings");
            gui->CallAfter([this]()
                           {
                            wxMessageDialog dial(NULL,
                                lcl.CONNECT_BINDINGS_ERROR, wxT("Error"), wxOK | wxICON_ERROR);
                            dial.ShowModal(); });
            status = DISCONNECTED;
            return;
        }
        vr::VRInput()->SetActionManifestPath(fullpath.c_str());

        vr::VRInput()->GetActionHandle("/actions/demo/in/grab_camera", &m_actionCamera);
        vr::VRInput()->GetActionHandle("/actions/demo/in/grab_trackers", &m_actionTrackers);
        vr::VRInput()->GetActionHandle("/actions/demo/in/Hand_Left", &m_actionHand);

        vr::VRInput()->GetActionSetHandle("/actions/demo", &m_actionsetDemo);
    }

    std::istringstream ret;
    std::string word;

    ret = Send("numtrackers");
    ret >> word;
    if (word != "numtrackers")
    {
        gui->CallAfter([this]()
                       {
                       wxMessageDialog dial(NULL,
                           lcl.CONNECT_DRIVER_ERROR, wxT("Error"), wxOK | wxICON_ERROR);
                       dial.ShowModal(); });
        status = DISCONNECTED;
        return;
    }
    int connected_trackers;
    ret >> connected_trackers;

    ret >> word;
    if (word != user_config.driver_version)
    {
        gui->CallAfter([this, word]()
            {
                std::string e = "";
                e += lcl.CONNECT_DRIVER_MISSMATCH_1 + word + lcl.CONNECT_DRIVER_MISSMATCH_2 + user_config.driver_version;
                wxMessageDialog dial(NULL,
                    e, wxT("Warning"), wxOK | wxICON_WARNING);
                dial.ShowModal();
            });
    }

    for (int i = connected_trackers; i < connectedTrackers.size(); i++)
    {
        ret = Send("addtracker " + connectedTrackers[i].Name + " " + connectedTrackers[i].Role);
        ret >> word;
        if (word != "added")
        {
            gui->CallAfter([this]()
                           {
                           wxMessageDialog dial(NULL,
                               lcl.CONNECT_SOMETHINGWRONG, wxT("Error"), wxOK | wxICON_ERROR);
                           dial.ShowModal(); });
            status = DISCONNECTED;
            return;
        }
    }

    ret = Send("addstation");

    ret = Send("settings 120 " + std::to_string(user_config.smoothingFactor) + " " + std::to_string(user_config.additionalSmoothing));

    //set that connection is established
    status = CONNECTED;
}

std::istringstream Connection::Send(std::string buffer)
{
    std::string resp;
    this->bridge_driver->send(buffer, resp);
    return std::istringstream(resp);
}

std::istringstream Connection::SendTracker(int id, double a, double b, double c, double qw, double qx, double qy, double qz, double time, double smoothing)
{

    std::string s;
    s = " updatepose " + std::to_string(id) +
        " " + std::to_string(a) +
        " " + std::to_string(b) +
        " " + std::to_string(c) +
        " " + std::to_string(qw) +
        " " + std::to_string(qx) +
        " " + std::to_string(qy) +
        " " + std::to_string(qz) +
        " " + std::to_string(time) +
        " " + std::to_string(smoothing) + "\n";

    // send the string to our driver

    return Send(s);
}

std::istringstream Connection::SendStation(int id, double a, double b, double c, double qw, double qx, double qy, double qz)
{

    std::string s;
    s = " updatestation " + std::to_string(id) +
        " " + std::to_string(a) +
        " " + std::to_string(b) +
        " " + std::to_string(c) +
        " " + std::to_string(qw) +
        " " + std::to_string(qx) +
        " " + std::to_string(qy) +
        " " + std::to_string(qz) + "\n";

    // send the string to our driver

    return Send(s);
}

bool GetDigitalActionState(vr::VRActionHandle_t action)
{
    vr::InputDigitalActionData_t actionData;
    if (vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle) != vr::VRInputError_None)
        return false;

    return actionData.bActive && actionData.bState;
}

int Connection::GetButtonStates()
{
    if (status == DISCONNECTED || user_config.disableOpenVrApi)
    {
        return 0;
    }

    vr::VRActiveActionSet_t actionSet = {0};
    actionSet.ulActionSet = m_actionsetDemo;
    vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);

    // vr::InputPoseActionData_t poseData;

    if (GetDigitalActionState(m_actionCamera))
        return 1;
    if (GetDigitalActionState(m_actionTrackers))
        return 2;

    return 0;
}

void Connection::GetControllerPose(double outpose[])
{
    if (user_config.disableOpenVrApi)
        return;

    // std::istringstream ret = Send("getdevicepose 1");
    // std::string word;

    // first three variables are a position vector
    // int idx;
    double a;
    double b;
    double c;

    // second four are rotation quaternion
    double qw;
    double qx;
    double qy;
    double qz;

    // read to our variables
    // ret >> word; ret >> idx; ret >> a; ret >> b; ret >> c; ret >> qw; ret >> qx; ret >> qy; ret >> qz;

    vr::VRActiveActionSet_t actionSet = {0};
    actionSet.ulActionSet = m_actionsetDemo;
    vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);

    vr::InputPoseActionData_t poseData;
    if (vr::VRInput()->GetPoseActionDataForNextFrame(m_actionHand, vr::TrackingUniverseRawAndUncalibrated, &poseData, sizeof(poseData), vr::k_ulInvalidInputValueHandle) != vr::VRInputError_None || !poseData.bActive || !poseData.pose.bPoseIsValid)
    {
        return;
    }
    else
    {
        vr::HmdMatrix34_t matrix = poseData.pose.mDeviceToAbsoluteTracking;

        qw = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
        qx = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
        qy = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
        qz = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
        qx = copysign(qx, matrix.m[2][1] - matrix.m[1][2]);
        qy = copysign(qy, matrix.m[0][2] - matrix.m[2][0]);
        qz = copysign(qz, matrix.m[1][0] - matrix.m[0][1]);

        a = matrix.m[0][3];
        b = matrix.m[1][3];
        c = matrix.m[2][3];
    }

    a = -a;
    c = -c;

    outpose[0] = a;
    outpose[1] = b;
    outpose[2] = c;
    outpose[3] = qw;
    outpose[4] = qx;
    outpose[5] = qy;
    outpose[6] = qz;
}
