#pragma once

#include <string>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#endif

namespace Ipc {

    class Connection {
        public:
            ~Connection();

            // Copying not allowed
            Connection(Connection const &) = delete;
            Connection& operator=(Connection const &) = delete;

            // Moving is allowed
            Connection(Connection &&) = default;
            Connection& operator=(Connection &&) = default;

            bool send(const char *buffer, size_t length);
            bool recv(char *buffer, size_t length);

        private:
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
            Connection(HANDLE inPipe);
            HANDLE inPipe;
#elif defined(__linux) || defined(__linux__) || defined(linux)
            Connection(int connfd);
            int connfd;
#else
            Connection();
#endif
            friend class Server;
    };

    class Server {
        public:
            Server();

            // Copying not allowed
            Server(Server const &) = delete;
            Server& operator=(Server const &) = delete;

            void init(std::string name);
            Connection accept();

        private:
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
            HANDLE inPipe;
#elif defined(__linux) || defined(__linux__) || defined(linux)
            int listenfd;
#endif
    };

    class Client {
        public:
            Client(std::string name);

            // Copying not allowed
            Client(Client const &) = delete;
            Client& operator=(Client const &) = delete;

            std::string sendrecv(std::string buffer);

        private:
            std::string name;
    };

}; // namespace Ipc
