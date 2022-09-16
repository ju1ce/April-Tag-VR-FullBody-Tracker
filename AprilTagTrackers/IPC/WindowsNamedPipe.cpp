#include "IPC.hpp"
#include "utils/Log.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace IPC
{

constexpr int SEC_TO_MS = 1000;

WindowsNamedPipe::WindowsNamedPipe(std::string pipeName)
    : mPipeName(R"(\\.\pipe\)" + std::move(pipeName)) {}

std::string_view WindowsNamedPipe::SendRecv(std::string_view message)
{
    // NOLINTNEXTLINE: Remove const-ness as callnamedpipe expects a void*, but it will not be modified
    LPVOID messagePtr = reinterpret_cast<LPVOID>(const_cast<char*>(message.data()));
    DWORD responseLength = 0;
    if (FAILED(CallNamedPipeA(
            mPipeName.c_str(),
            messagePtr,
            message.size(),
            GetBufferPtr(),
            BUFFER_SIZE,
            &responseLength,
            2 * SEC_TO_MS)))
    {
        ATT_LOG_ERROR("named pipe send error: ", GetLastError());
        throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
    }
    return GetBufferStringView(static_cast<int>(responseLength));
}

} // namespace IPC

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
