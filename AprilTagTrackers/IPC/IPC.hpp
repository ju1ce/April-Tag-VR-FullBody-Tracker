#pragma once

#include "utils/Assert.hpp"
#include "utils/Types.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

namespace IPC
{

template <Index NSize>
class IOBuffer
{
public:
protected:
    constexpr std::string_view GetBufferStringView(Index stringLength) const
    {
        ATT_ASSERT(stringLength >= 0);
        ATT_ASSERT(stringLength <= NSize);
        return std::string_view(mBuffer.data(), stringLength);
    }
    constexpr char* GetBufferPtr() { return mBuffer.data(); }
    constexpr Index GetBufferSize() const { return NSize - 1; }

private:
    std::array<char, NSize> mBuffer{};
};

class IConnection : public IOBuffer<1024>
{
public:
    virtual ~IConnection() = default;
    virtual void Send(std::string_view message) = 0;
    /// @return temporary view of buffer, invalidated when SendRecv is called again
    [[nodiscard]] virtual std::string_view Recv() = 0;
};

// Interface for inter-process-communication, be that over network, udp, or pipes, multithreaded or not
class IServer
{
public:
    virtual ~IServer() = default;
    [[nodiscard]] virtual std::unique_ptr<IConnection> Accept() = 0;
};

// Interface for inter-process-communication, be that over network, udp, or pipes, multithreaded or not
class IClient : public IOBuffer<1024>
{
public:
    virtual ~IClient() = default;
    /// @return temporary view of buffer, invalidated when SendRecv is called again
    [[nodiscard]] virtual std::string_view SendRecv(std::string_view message) = 0;
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

inline std::unique_ptr<IClient> CreateDriverClient()
{
    const std::string driverPath = "AprilTagPipeIn";
#ifdef ATT_OS_WINDOWS
    return std::make_unique<WindowsNamedPipe>(driverPath);
#else
    return std::make_unique<UNIXSocket>(driverPath);
#endif
}

[[nodiscard]] std::unique_ptr<IServer> CreateDriverServer();

}; // namespace IPC
