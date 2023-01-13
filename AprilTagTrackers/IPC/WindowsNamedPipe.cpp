#ifdef ATT_OS_WINDOWS

#    include "IPC.hpp"
#    include "utils/Error.hpp"
#    include "utils/Log.hpp"

#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <Windows.h>

namespace IPC
{

constexpr int SEC_TO_MS = 1000;

WindowsNamedPipe::WindowsNamedPipe(std::string pipeName)
    : mPipeName(R"(\\.\pipe\)" + std::move(pipeName)) {}

std::string_view WindowsNamedPipe::SendRecv(std::string message)
{
    // NOLINTNEXTLINE: Remove const-ness as callnamedpipe expects a void*, but it will not be modified
    LPVOID messagePtr = reinterpret_cast<LPVOID>(const_cast<char*>(message.data()));
    DWORD responseLength = 0;
    if (FAILED(CallNamedPipeA(
            mPipeName.c_str(),
            messagePtr,
            message.size() + 1,
            GetBufferPtr(),
            GetBufferSize(),
            &responseLength,
            2 * SEC_TO_MS)))
    {
        ATT_LOG_ERROR("named pipe send error: ", GetLastError());
        throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
    }
    return GetBufferStringView(static_cast<int>(responseLength));
}

namespace
{

class WindowsNamedPipeConnection final : public IConnection
{
public:
    explicit WindowsNamedPipeConnection(HANDLE pipe) : mPipe(pipe) {}
    ~WindowsNamedPipeConnection() final
    {
        if (mPipe == nullptr) return;
        if (FAILED(DisconnectNamedPipe(mPipe))) ATT_LOG_ERROR("disconnect named pipe: ", GetLastError());
    }

    void Send(std::string_view message) final
    {
        DWORD bytesWritten = 0;
        if (FAILED(WriteFile(mPipe, message.data(), message.size(), &bytesWritten, nullptr)))
        {
            throw utils::MakeError("named pipe write file: ", GetLastError());
        }
    }

    std::string_view Recv() final
    {
        DWORD bytesRead = 0;
        if (FAILED(ReadFile(mPipe, GetBufferPtr(), GetBufferSize(), &bytesRead, nullptr)))
        {
            throw utils::MakeError("named pipe read file: ", GetLastError());
        }
        // ATT_LOG_DEBUG("recv(", bytesRead, "): ", std::string_view(GetBufferPtr(), 20));
        return GetBufferStringView(bytesRead);
    }

private:
    HANDLE mPipe = nullptr;
};

class WindowsNamedPipeServer final : public IServer
{
public:
    explicit WindowsNamedPipeServer(std::string pipeName)
    {
        pipeName = R"(\\.\pipe\)" + pipeName;
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        mPipe = CreateNamedPipeA(pipeName.c_str(),
                                 PIPE_ACCESS_DUPLEX,
                                 // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
                                 PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                 1,
                                 1024 * 16,
                                 1024 * 16,
                                 NMPWAIT_USE_DEFAULT_WAIT,
                                 nullptr);
        if (mPipe == nullptr) throw utils::MakeError("create named pipe: ", GetLastError());
    }

    /// connection is only valid for one recv and send, then disconnect must be called and a new connection accepted
    std::unique_ptr<IConnection> Accept() final
    {
        if (FAILED(ConnectNamedPipe(mPipe, nullptr))) throw utils::MakeError("connect named pipe: ", GetLastError());
        return std::make_unique<WindowsNamedPipeConnection>(mPipe);
    }

private:
    HANDLE mPipe = nullptr;
};

} // namespace

[[nodiscard]] std::unique_ptr<IServer> CreateDriverServer()
{
    return std::make_unique<WindowsNamedPipeServer>("AprilTagPipeIn");
}

} // namespace IPC

#else
#    error WindowsNamedPipe.cpp only compiled on windows
#endif
