#include "Connection.h"


Connection::Connection(Parameters* params)
{
    parameters = params;
}

void Connection::StartConnection()
{
    if (status == WAITING)
    {
        wxMessageDialog dial(NULL,
            wxT("Already waiting for a connection"), wxT("Error"), wxOK | wxICON_ERROR);
        dial.ShowModal();
        return;
    }
    if (status == CONNECTED)
    {
        wxMessageDialog dial(NULL,
            wxT("Already connected. Restart connection?"), wxT("Question"),
            wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
        if (dial.ShowModal() != wxID_YES)
        {
            return;
        }
        Sleep(1000);
        status = DISCONNECTED;
    }
    std::thread connectThread(&Connection::Connect, this);
    connectThread.detach();
}

void Connection::Connect()
{
    //generate vector of tracker connection struct, connecting board ids to apropriate driver ids. In future, this should be done manualy in the gui
    connectedTrackers.clear();

    if (parameters->ignoreTracker0)
    {
        for (int i = 0; i < parameters->trackerNum - 1; i++)
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
    else
    {
        for (int i = 0; i < parameters->trackerNum; i++)
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

    //connect to steamvr as a client in order to get buttons.
    vr::EVRInitError error;
    openvr_handle = VR_Init(&error, vr::VRApplication_Overlay);

    DWORD  retval = 0;
    BOOL   success;
    char  buffer[1024] = "";
    char** lppPart = { NULL };

    retval = GetFullPathNameA("att_actions.json",
        BUFSIZE,
        buffer,
        lppPart);

    vr::VRInput()->SetActionManifestPath(buffer);

    vr::VRInput()->GetActionHandle("/actions/demo/in/grab_camera", &m_actionCamera);
    vr::VRInput()->GetActionHandle("/actions/demo/in/grab_trackers", &m_actionTrackers);

    vr::VRInput()->GetActionSetHandle("/actions/demo", &m_actionsetDemo);

    std::istringstream ret;
    std::string word;

    ret = Send("numtrackers");
    ret >> word;
    if (word != "numtrackers")
    {
        wxMessageDialog dial(NULL,
            wxT("Could not connect to SteamVR driver. Make sure SteamVR is running and the apriltagtrackers driver is installed. \nYou may also have to run bin/ApriltagTrackers.exe as administrator, if error code is not 2. \nError code: " + std::to_string(GetLastError())), wxT("Error"), wxOK | wxICON_ERROR);
        dial.ShowModal();
        status = DISCONNECTED;
        return;
    }
    int connected_trackers;
    ret >> connected_trackers;
    for (int i = connected_trackers; i < connectedTrackers.size(); i++)
    {
        ret = Send("addtracker " + connectedTrackers[i].Name + " " + connectedTrackers[i].Role);
        ret >> word;
        if (word != "added")
        {
            wxMessageDialog dial(NULL,
                wxT("Something went wrong. Try again."), wxT("Error"), wxOK | wxICON_ERROR);
            dial.ShowModal();
            status = DISCONNECTED;
            return;
        }
    }
    ret = Send("addstation");

    ret = Send("settings 50 " + std::to_string(parameters->smoothingFactor));           //TODO: set the parameters correctly. I is lazy

    //set that connection is established
    status = CONNECTED;
}

std::istringstream Connection::Send(std::string lpszWrite)
{
    //function expecting LPWGSTR instead of LPCASDFGEGTFSTR you are passing? I have no bloody clue what any of that even means. It works for me, so I'll leave the dumb conversions and casts in. If it doesn't for you, have fun.

    fSuccess = CallNamedPipe(
        lpszPipename,        // pipe name
        (LPVOID)lpszWrite.c_str(),           // message to server
        (strlen(lpszWrite.c_str()) + 1) * sizeof(TCHAR), // message length
        chReadBuf,              // buffer to receive reply
        BUFSIZE * sizeof(TCHAR),  // size of read buffer
        &cbRead,                // number of bytes read
        2000);                 // waits for 2 seconds

    if (fSuccess || GetLastError() == ERROR_MORE_DATA)
    {
        std::cout << chReadBuf << std::endl;
        chReadBuf[cbRead] = '\0'; //add terminating zero
                    //convert our buffer to string
        std::string rec = chReadBuf;
        std::istringstream iss(rec);
        // The pipe is closed; no more data can be read.

        if (!fSuccess)
        {
            printf("\nExtra data in message was lost\n");
        }
        return iss;
    }
    else
    {
        std::cout << GetLastError() << " :(" << std::endl;
        std::string rec = " senderror";
        std::istringstream iss(rec);
        return iss;
    }
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

    //send the string to our driver

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

    //send the string to our driver

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
    if (status == DISCONNECTED)
    {
        return 0;
    }

    vr::VRActiveActionSet_t actionSet = { 0 };
    actionSet.ulActionSet = m_actionsetDemo;
    vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);

    if (GetDigitalActionState(m_actionCamera))
        return 1;
    if (GetDigitalActionState(m_actionTrackers))
        return 2;

    return 0;
}

void Connection::GetControllerPose(double outpose[])
{
    std::istringstream ret = Send("getdevicepose 1");
    std::string word;

    //first three variables are a position vector
    int idx; double a; double b; double c;

    //second four are rotation quaternion
    double qw; double qx; double qy; double qz;

    //read to our variables
    ret >> word; ret >> idx; ret >> a; ret >> b; ret >> c; ret >> qw; ret >> qx; ret >> qy; ret >> qz;

    a = -a;
    c = -c;

    outpose[0] = a; outpose[1] = b; outpose[2] = c; outpose[3] = qw; outpose[4] = qx; outpose[5] = qy; outpose[6] = qz;
}
