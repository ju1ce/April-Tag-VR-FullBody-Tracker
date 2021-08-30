#include <sstream>

#if defined(__linux) || defined(__linux__) || defined(linux)
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include "Ipc.hpp"

namespace Ipc {

    const int BUFSIZE = 1024;

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    Connection::Connection(HANDLE inPipe) : inPipe(inPipe) { }

    Connection::~Connection()
    {
        DisconnectNamedPipe(inPipe);
    }

    bool Connection::send(const char *buffer, size_t length)
    {
        DWORD dwWritten;

        if (WriteFile(inPipe, buffer, length, &dwWritten, NULL) != FALSE)
        {
            return true;
        }
        else
        {
            return false
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
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE |PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
            1,
            1024 * 16,
            1024 * 16,
            NMPWAIT_USE_DEFAULT_WAIT,
            NULL);
    }

    Connection Server::accept()
    {
        ConnectNamedPipe(inPipe, NULL);
        return Connection(inPipe);
    }

    Client::Client(std::string name) : name(name) { }

    std::string Client::sendrecv(std::string buffer)
    {
        CHAR chReadBuf[BUFSIZE];
        BOOL fSuccess;
        DWORD cbRead;

        std::stringstream ss;
        ss << "\\\\.\\pipe\\" << name;
        std::string outPipeName = ss.str();

        fSuccess = CallNamedPipeA(
            outPipeName.c_str(),        // pipe name 
            buffer.c_str(),           // message to server 
            (buffer.size() + 1), // message length 
            chReadBuf,              // buffer to receive reply 
            BUFSIZE,  // size of read buffer 
            &cbRead,                // number of bytes read 
            2000);                 // waits for 2 seconds 

        if (fSuccess || GetLastError() == ERROR_MORE_DATA)
        {
            chReadBuf[cbRead] = '\0'; //add terminating zero
            std::cout << readBuffer << std::endl;
            // The pipe is closed; no more data can be read. 

            std::string ret = chReadBuf;

            if (!fSuccess)
            {
                printf("\nExtra data in message was lost\n");
            }
            return ret;
        }
        else
        {
            std::cout << GetLastError() << " :(" << std::endl;
            std::string ret = " senderror";
            return ret;
        }
    }

#elif defined(__linux) || defined(__linux__) || defined(linux)

    Server::Server() { }

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

    Client::Client(std::string name) : name(name) { }

    std::string Client::sendrecv(std::string buffer)
    {
        int s, t, len;
        struct sockaddr_un remote;

        if ((s = ::socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1) {
            perror("socket");
            return std::string();
        }

        std::stringstream ss;
        ss << "/tmp/" << name;
        std::string sockpath = ss.str();

        remote.sun_family = AF_UNIX;
        strcpy(remote.sun_path, sockpath.c_str());
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);
        if (::connect(s, (struct sockaddr *)&remote, len) == -1) {
            perror("connect");
            close(s);
            return std::string();
        }

        if (::send(s, buffer.c_str(), buffer.size() + 1, 0) == -1) {
            perror("send");
            close(s);
            return std::string();
        }

        char readBuf[BUFSIZE];

        if ((t = ::recv(s, readBuf, BUFSIZE, 0)) > 0) {
            readBuf[t] = '\0';
            close(s);
            return std::string(readBuf);
        }
        else {
            perror("recv");
            close(s);
            return std::string();
        }
    }

#else

    Server::Server() { }
    void Server::init(std::string name) { }
    Connection Server::accept()
    {
        return Connection();
    }

    Connection::Connection() { }
    Connection::~Connection() { }
    bool Connection::send(const char *buf, size_t len)
    {
        return false;
    }
    bool Connection::recv(char *buf, size_t len)
    {
        return false;
    }

    Client::Client(std::string name) : name(name) { }

    std::string Client::sendrecv(std::string buffer) { return std::string(); }

#endif

}; // namespace Ipc