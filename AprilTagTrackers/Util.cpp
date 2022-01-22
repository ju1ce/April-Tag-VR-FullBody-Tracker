#include "Util.h"

#include <Windows.h>

void sleep_millis(int duration)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
}

bool get_full_path(const std::string& filename, std::string& out_full_path)
{
#if OS_WIN
    constexpr int BUFFER_MAX = 512;
    char buffer[BUFFER_MAX];
    unsigned int path_length = 0;

    path_length = GetFullPathNameA(filename.c_str(), BUFFER_MAX, buffer, NULL);
    if (path_length > BUFFER_MAX || path_length == 0) {
        return false;
    }
    out_full_path = std::string(buffer, path_length);
    return true;
#elif OS_LINUX
    const char* path = realpath(filename.c_str(), NULL);
    if (path == NULL) return false;
    out_full_path = std::string(path);
    return true;
#endif
}