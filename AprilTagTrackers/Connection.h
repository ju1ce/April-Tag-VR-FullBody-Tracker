#pragma once
#pragma warning(push)
#pragma warning(disable : 4996)
#include <wx/wx.h>
#pragma warning(pop)

#include "Config.h"
#include "IPC/IPC.h"
#include "Util.h"
#include "i18n.h"
#include <memory>
#include <openvr.h>

struct TrackerConnection
{
    int TrackerId;
    int DriverId;
    std::string Name;
    std::string Role;
};

class GUI;

class Connection
{
public:
    enum ConnectionStatus
    {
        DISCONNECTED,
        WAITING,
        CONNECTED
    };

    Connection(const UserConfig& user_config, const Localization& lcl);
    void StartConnection();
    std::istringstream Send(std::string buffer);
    std::istringstream SendTracker(int id, double a, double b, double c, double qw, double qx, double qy, double qz, double time, double smoothing);
    std::istringstream SendStation(int id, double a, double b, double c, double qw, double qx, double qy, double qz);
    void GetControllerPose(double outpose[]);
    int GetButtonStates();

    ConnectionStatus status = DISCONNECTED;
    GUI* gui = nullptr;
    vr::IVRSystem* openvr_handle = nullptr;
    std::vector<TrackerConnection> connectedTrackers;

private:
    void Connect();

    const UserConfig& user_config;
    const Localization& lcl;
    std::unique_ptr<IPC::IClient> bridge_driver;

    vr::VRActionHandle_t m_actionCamera = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionsetDemo = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionTrackers = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionHand = vr::k_ulInvalidActionHandle;
};
