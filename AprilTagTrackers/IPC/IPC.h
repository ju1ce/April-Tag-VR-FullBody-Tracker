#pragma once

#include <memory>
#include <string>

namespace IPC
{

// Interface for inter-process-communication, be that over network, udp, or pipes, multithreaded or not
class IServer
{
public:
    virtual ~IServer() = default;
    void (*on_message)(std::string message) = nullptr;
};

// Interface for inter-process-communication, be that over network, udp, or pipes, multithreaded or not
class IClient
{
public:
    virtual ~IClient() = default;
    // returns true on success
    virtual bool send(const std::string &message, std::string &out_response) = 0;
};

class WindowsNamedPipe : public IClient
{
public:
    explicit WindowsNamedPipe(const std::string &pipe_name);

    bool send(const std::string &msg, std::string &resp) override;

private:
    std::string pipe_name;
};

class UNIXSocket : public IClient
{
public:
    explicit UNIXSocket(const std::string &socket_name);

    bool send(const std::string &msg, std::string &resp) override;

private:
    std::string socket_path;
};

};