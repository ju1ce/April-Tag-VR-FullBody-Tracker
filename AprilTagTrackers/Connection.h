#pragma once
#pragma warning(push)
#pragma warning(disable:4996)
#include <wx/wx.h>
#pragma warning(pop)

#include "Parameter.h"
#include <openvr.h>
#include <memory>

struct TrackerConnection {
    int TrackerId;
    int DriverId;
    std::string Name;
    std::string Role;
};

#include "Util.h"

#include "IPC/IPC.h"

class GUI;

class Connection
{
public:
    const int DISCONNECTED = 0;
    const int WAITING = 1;
    const int CONNECTED = 2;
    Connection(Parameters*);
    Parameters* parameters;
    void StartConnection();
    std::istringstream Send(std::string buffer);
    std::istringstream SendTracker(int id, double a, double b, double c, double qw, double qx, double qy, double qz, double time, double smoothing);
    std::istringstream SendStation(int id, double a, double b, double c, double qw, double qx, double qy, double qz);
    void GetControllerPose(double outpose[]);
    int GetButtonStates();
    int status = DISCONNECTED;
    vr::IVRSystem* openvr_handle;
    std::vector<TrackerConnection> connectedTrackers;
    bool disableOpenVrApi = true;
    GUI* gui;
private:
    void Connect();
    std::unique_ptr<IPC::IClient> bridge_driver;
    vr::VRActionHandle_t m_actionCamera = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionsetDemo = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionTrackers = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionHand = vr::k_ulInvalidActionHandle;
    
};
