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
    //function to create pipes for SteamVR connection
    pipeNum = parameters->trackerNum;

    double trackerNum = parameters->trackerNum;
    if (parameters->ignoreTracker0)
    {
        trackerNum--;
    }

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
    for (int i = connected_trackers; i < trackerNum; i++)
    {
        ret = Send("addtracker");
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

    ret = Send("settings " + std::to_string(parameters->smoothingFactor) + " 2");

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
    if (parameters->ignoreTracker0) {
        id--;
    }

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
