#if OS_WIN
#include "IPC.h"

#include <Windows.h>
#include <iostream>

namespace IPC
{

constexpr int BUFFER_SIZE = 512;
constexpr int SEC_TO_MS = 1000;

WindowsNamedPipe::WindowsNamedPipe(const std::string& pipe_name) {
    this->pipe_name = "\\\\.\\pipe\\" + pipe_name;
}

bool WindowsNamedPipe::send(const std::string& msg, std::string& resp) {
    char response_buffer[BUFFER_SIZE];
    DWORD response_length = 0;
    // Remove const-ness as callnamedpipe expects a void*, it will not change the inbuffer.
    auto msg_cstr = reinterpret_cast<LPVOID>(const_cast<char*>(msg.c_str()));
    // success will be zero if failed
    if (FAILED(CallNamedPipeA(
        this->pipe_name.c_str(), // pipe name
        msg_cstr, // message
        msg.size(), // message size
        response_buffer, // response
        BUFFER_SIZE, // response max size
        &response_length, // response size
        2 * SEC_TO_MS))) // timeout in ms
    {
        std::cerr << "Named pipe (" << this->pipe_name << ") send error: " << GetLastError() << std::endl;
        return false;
    }

    resp = std::string(response_buffer, response_length);
    return true;
}

}
#endif

/*

    bool Connection::send(const char *buffer, size_t length)
    {
        DWORD dwWritten;

        if (WriteFile(inPipe, buffer, length, &dwWritten, NULL) != FALSE)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Connection::recv(char *buffer, size_t length)
    {
        DWORD dwRead;

        if (ReadFile(inPipe, buffer, length - 1, &dwRead, NULL) != FALSE)
        {
            buffer[dwRead] = '\0'; //add terminating zero
            return true;
        }
        else
        {
            return false;
        }
    }


    Server::Server() { }

    void Server::init(std::string name)
    {
        std::stringstream ss;
        ss << "\\\\.\\pipe\\" << name;
        std::string inPipeName = ss.str();

        inPipe = CreateNamedPipeA(inPipeName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
            1,
            1024 * 16,
            1024 * 16,
            NMPWAIT_USE_DEFAULT_WAIT,
            NULL);
    }

    */
