#pragma once

#include "../Util.h"

#if OS_WIN

#include "IIPC.h"

namespace IPC
{

class WindowsNamedPipe : public IClient
{
public:
    WindowsNamedPipe(const std::string& pipe_name);
    
    bool send(const std::string& msg, std::string& resp) override;
private:
    std::string pipe_name;
};

};

#endif