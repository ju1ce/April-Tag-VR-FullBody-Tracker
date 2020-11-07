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
    void Send(int,double, double, double, double, double, double, double);
    int status = DISCONNECTED;
private:
    void Connect();
    std::vector<HANDLE> hpipe;
    int pipeNum = 1;
    char buffer[1024];
    DWORD dwRead;
    DWORD dwWritten;

};

