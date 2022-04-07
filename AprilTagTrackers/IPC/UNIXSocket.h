#pragma once

#include "../Util.h"

#if OS_LINUX
#include "IIPC.h"

namespace IPC
{

class UNIXSocket : public IClient
{
public:
    UNIXSocket(const std::string& socket_name);

    bool send(const std::string& msg, std::string& resp) override;
private:
    std::string socket_path;
};

}

#endif