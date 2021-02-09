#pragma once
#include <wx/wx.h>
#include "Parameters.h"
#include <windows.h> 

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
    std::istringstream SendTracker(int id, double a, double b, double c, double qw, double qx, double qy, double qz, double time);
    std::istringstream SendStation(int id, double a, double b, double c, double qw, double qx, double qy, double qz);
    int status = DISCONNECTED;
private:
    void Connect();
    HANDLE hpipe;
    int pipeNum = 1;
    const int BUFSIZE = 1024;
    CHAR chReadBuf[1024];
    BOOL fSuccess;
    DWORD cbRead;
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\ApriltagPipeIn");

};

