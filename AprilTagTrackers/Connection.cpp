#include "Connection.h"

Connection::Connection(Parameters* params)
{
    parameters = params;
}

void Connection::StartConnection()
{	
	if (status == WAITING)
	{
		wxMessageDialog* dial = new wxMessageDialog(NULL,
			wxT("Already waiting for a connection"), wxT("Error"), wxOK | wxICON_ERROR);
		dial->ShowModal();
		return;
	}
	if (status == CONNECTED)
	{
		wxMessageDialog* dial = new wxMessageDialog(NULL,
			wxT("Already connected. Restart connection?"), wxT("Question"),
			wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
		if (dial->ShowModal() != wxID_YES)
		{
			return;
		}
		for (int i = 0; i < hpipe.size(); i++)
		{
			DisconnectNamedPipe(hpipe[i]);
			CloseHandle(hpipe[i]);
		}
		hpipe.clear();
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

	status = WAITING;

	for (int i = 0; i < pipeNum; i++)
	{
		//create a pipe with given name and index
		int index = parameters->ignoreTracker0 ? i - 1 : i;
		std::string pipeName = "\\\\.\\pipe\\TrackPipe" + std::to_string(index);
		HANDLE pipe;
		if(i != 0 || !parameters->ignoreTracker0)
		{
			pipe = CreateNamedPipeA(pipeName.c_str(),
				PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
				1,
				1024 * 16,
				1024 * 16,
				NMPWAIT_USE_DEFAULT_WAIT,
				NULL);
			if (pipe != INVALID_HANDLE_VALUE)
			{
				//if pipe was successfully created wait for a connection
				if (ConnectNamedPipe(pipe, NULL) != FALSE)   // wait for someone to connect to the pipe
				{
					//when pipe is connected, send number of pipes and driversmoothfactor to our connected driver
					std::string s = std::to_string(trackerNum) + " 0";

					//write our data to pipe
					WriteFile(pipe,
						s.c_str(),
						(s.length() + 1),   // = length of string + terminating '\0' !!!
						&dwWritten,
						NULL);

				}
			}
			else
			{
				wxMessageDialog* dial = new wxMessageDialog(NULL,
					wxT("Could not start connection"), wxT("Error"), wxOK | wxICON_ERROR);
				dial->ShowModal();
				status = DISCONNECTED;
				return;
			}
		}
		//add our pipe to our global list of pipes
		hpipe.push_back(pipe);
	}
	//set that connection is established
	status = CONNECTED;
}

void Connection::Send(int id, double a, double b, double c, double qw, double qx, double qy, double qz)
{
	if (status != CONNECTED && id >= hpipe.size())
	{
		return;
	}
	std::string s;
	s = std::to_string(a) +
		" " + std::to_string(b) +
		" " + std::to_string(c) +
		" " + std::to_string(qw) +
		" " + std::to_string(qx) +
		" " + std::to_string(qy) +
		" " + std::to_string(qz) + "\n";

	//send the string to our driver

	WriteFile(hpipe[id],
		s.c_str(),
		(s.length() + 1),   // = length of string + terminating '\0' !!!
		&dwWritten,
		NULL);
}