#pragma once

#include "utils/Assert.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

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
    /// @return temporary view of IClient buffer, invalidated when SendRecv is called again
    [[nodiscard]] virtual std::string_view SendRecv(std::string_view message) = 0;

protected:
    static constexpr int BUFFER_SIZE = 1024;
    constexpr std::string_view GetBufferStringView(int stringLength) const
    {
        ATT_ASSERT(stringLength > 0);
        ATT_ASSERT(stringLength <= BUFFER_SIZE);
        return {mBuffer.data(), static_cast<std::size_t>(stringLength)};
    }
    constexpr char* GetBufferPtr() { return mBuffer.data(); }

private:
    std::array<char, BUFFER_SIZE> mBuffer;
};

class WindowsNamedPipe : public IClient
{
public:
    explicit WindowsNamedPipe(std::string pipeName);

    std::string_view SendRecv(std::string_view message) final;

private:
    std::string mPipeName;
};

class UNIXSocket : public IClient
{
public:
    explicit UNIXSocket(std::string socketName);

    std::string_view SendRecv(std::string_view message) final;

private:
    std::string mSocketPath;
};

inline std::unique_ptr<IClient> CreateDriverConnection()
{
    const std::string driverPath = "AprilTagPipeIn";
#ifdef ATT_OS_WINDOWS
    return std::make_unique<WindowsNamedPipe>(driverPath);
#else
    return std::make_unique<UNIXSocket>(driverPath);
#endif
}

}; // namespace IPC
