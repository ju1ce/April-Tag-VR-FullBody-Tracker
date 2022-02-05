#pragma once

#include <string>
#include <memory>

#include "Util.h"

namespace IPC
{

// Interface for inter-process-communication, be that over network, udp, or pipes, multithreaded or not 
class IServer
{
public:
    virtual ~IServer() {};

    void (*on_message)(std::string message) = 0;
};

// Interface for inter-process-communication, be that over network, udp, or pipes, multithreaded or not 
class IClient
{
public:
    virtual ~IClient() {};

    // returns true on success
    virtual bool send(const std::string& message, std::string& out_response) = 0;
};

#if OS_WIN
class WindowsNamedPipe : public IClient
{
public:
    WindowsNamedPipe(const std::string& pipe_name);
    
    bool send(const std::string& msg, std::string& resp) override;
private:
    std::string pipe_name;
};
#endif

#if OS_LINUX
class UNIXSocket : public IClient
{
public:
    UNIXSocket(const std::string& socket_name);

    bool send(const std::string& msg, std::string& resp) override;
private:
    std::string socket_path;
};
#endif

};