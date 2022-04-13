#if OS_LINUX
#include "IPC.h"

#include <errno.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace IPC
{

constexpr int BUFFER_SIZE = 512;

UNIXSocket::UNIXSocket(const std::string& socket_name)
{
    this->socket_path = "/tmp/" + socket_name;
}

bool UNIXSocket::send(const std::string& msg, std::string& resp)
{
    int socket = ::socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (socket == -1)
    {
        std::cerr << "Failed to create socket." << std::endl;
        return false;
    }

    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, this->socket_path.c_str());
    unsigned int addr_size = sizeof(server_addr.sun_family) + socket_path.size();
    if (::connect(socket, reinterpret_cast<struct sockaddr*>(&server_addr), addr_size) == -1)
    {
        std::cerr << "Failed to connect to socket " << this->socket_path << std::endl;
        close(socket);
        return false;
    }

    if (::send(socket, msg.c_str(), msg.size(), 0) == -1)
    {
        std::cerr << "Failed to send buffer to socket " << this->socket_path << std::endl;
        close(socket);
        return false;
    }

    char response_buffer[BUFFER_SIZE];
    unsigned long response_length = 0;
    if ((response_length = ::recv(socket, response_buffer, BUFFER_SIZE, 0)) == 0) {
        std::cerr << "Failed to receive response from socket " << this->socket_path << std::endl;
        close(socket);
        return false;
    }

    close(socket);
    resp = std::string(response_buffer, response_length);
    return true;
}

}

#endif

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
