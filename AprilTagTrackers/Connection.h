#pragma once
#pragma warning(push)
#pragma warning(disable:4996)
#include <wx/wx.h>
#pragma warning(pop)

#include "Parameters.h"
#include <windows.h>
#include <openvr.h>

class Connection
{
public:
    const int DISCONNECTED = 0;
    const int WAITING = 1;
    const int CONNECTED = 2;
    Connection(Parameters*);
    Parameters* parameters;
    void StartConnection();
    std::istringstream Send(std::string lpszWrite);
    std::istringstream SendTracker(int id, double a, double b, double c, double qw, double qx, double qy, double qz, double time, double smoothing);
    std::istringstream SendStation(int id, double a, double b, double c, double qw, double qx, double qy, double qz);
    void GetControllerPose(double outpose[]);
    int GetButtonStates();
    int status = DISCONNECTED;
    vr::IVRSystem* openvr_handle;
private:
    void Connect();
    HANDLE hpipe;
    int pipeNum = 1;
    const int BUFSIZE = 1024;
    CHAR chReadBuf[1024];
    BOOL fSuccess;
    DWORD cbRead;
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\ApriltagPipeIn");

    vr::VRActionHandle_t m_actionCamera = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionsetDemo = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionTrackers = vr::k_ulInvalidActionHandle;
};
