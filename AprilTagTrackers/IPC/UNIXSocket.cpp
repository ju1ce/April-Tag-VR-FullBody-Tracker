#ifdef ATT_OS_LINUX

#    include "IPC.hpp"

#    include <sys/socket.h>
#    include <sys/types.h>
#    include <sys/un.h>
#    include <unistd.h>

#    include <cerrno>
#    include <cstring>
#    include <system_error>
#    include <tuple>

using std::size_t;
using sockaddr_un_t = struct ::sockaddr_un;
using sockaddr_t = struct ::sockaddr;

namespace
{

template <typename TFn, typename... TArgs>
inline int SysCall(TFn&& func, TArgs&&... args)
{
    const int result = func(std::forward<TArgs>(args)...);
    if (result == -1) throw std::system_error(errno, std::generic_category());
    ATT_ASSERT(result >= 0);
    return result;
}

std::tuple<sockaddr_un_t, socklen_t> CreateAddress(std::string_view path)
{
    constexpr int maxPathSize = sizeof(sockaddr_un_t::sun_path);
    ATT_ASSERT(path.size() < maxPathSize);

    sockaddr_un_t serverAddr{};
    serverAddr.sun_family = AF_UNIX;
    std::strncpy(serverAddr.sun_path, path.data(), path.size());
    serverAddr.sun_path[path.size()] = '\0';

    constexpr int pathOffset = sizeof(sockaddr_un_t) - maxPathSize;
    // start of struct + path length + null byte
    const socklen_t addrSize = pathOffset + path.size() + 1;
    return {serverAddr, addrSize};
}

size_t SendRecv(std::string_view path, std::string_view message, char* bufferPtr, int bufferSize)
{
    const int socketFD = SysCall(::socket, AF_UNIX, SOCK_SEQPACKET, 0);
    try
    {
        const auto [serverAddr, addrSize] = CreateAddress(path);
        SysCall(::connect, socketFD, reinterpret_cast<const sockaddr_t*>(&serverAddr), addrSize); // NOLINT: cast necessary
        SysCall(::send, socketFD, message.data(), message.size() + 1, 0);
        const std::size_t responseLength = SysCall(::recv, socketFD, bufferPtr, bufferSize, 0);
        ::close(socketFD);
        return responseLength;
    }
    catch (...)
    {
        ::close(socketFD);
        throw;
    }
}

} // namespace

namespace IPC
{

UNIXSocket::UNIXSocket(std::string socketName)
    : mSocketPath("/tmp/" + std::move(socketName)) {}

std::string_view UNIXSocket::SendRecv(std::string_view message)
{
    try
    {
        const std::size_t responseLength = ::SendRecv(mSocketPath, message, GetBufferPtr(), GetBufferSize());
        return GetBufferStringView(static_cast<int>(responseLength));
    }
    catch (const std::system_error& e)
    {
        ATT_LOG_ERROR("socket error: ", e.what());
        throw;
    }
}

[[nodiscard]] std::unique_ptr<IServer> CreateDriverServer()
{
    utils::Unreachable(); // unimplemented
}

} // namespace IPC

/*

void Server::init(std::string name)
    {
        struct sockaddr_un local;

        if ((listenfd = ::socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1) {
            perror("socket");
        }

        std::stringstream ss;
        ss << "/tmp/" << name;
        std::string sockpath = ss.str();

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, sockpath.c_str());
        unlink(local.sun_path);
        int len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (::bind(listenfd, (struct sockaddr *)&local, len) == -1) {
            perror("bind");
        }

        if (::listen(listenfd, 5) == -1) {
            perror("listen");
        }
    }

    Connection Server::accept()
    {
        struct sockaddr_un remote;
        int connfd;
        socklen_t t = sizeof(remote);
        if ((connfd = ::accept(listenfd, (struct sockaddr *)&remote, &t)) == -1) {
            perror("accept");
        }
        return Connection(connfd);
    }

    Connection::Connection(int connfd) : connfd(connfd) { }

    Connection::~Connection()
    {
        ::close(connfd);
    }

    bool Connection::send(const char *buf, size_t len)
    {
        bool ret = true;

        if (::send(connfd, buf, len, 0) < 0) {
            perror("send");
            ret = false;
        }

        return ret;
    }

    bool Connection::recv(char *buf, size_t len)
    {
        bool ret = true;

        int n;
        n = ::recv(connfd, buf, len, 0);
        if (n < 0) {
            perror("recv");
            ret = false;
        }

        return ret;
    }

    */
#endif
