#pragma once

#include <string>
#include <memory>

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

};