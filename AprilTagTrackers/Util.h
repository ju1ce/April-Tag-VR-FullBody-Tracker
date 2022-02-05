#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__CYGWIN__)
    #define OS_WIN 1
#elif defined(unix) || defined(__unix__)
    #define OS_LINUX 1
#elif defined(__APPLE__)
    #define OS_MACOS 1
    #error Building on MacOS is not currently supported
#else
    #error Unknown OS
#endif

#include <string>

void sleep_millis(int);
bool get_full_path(const std::string& filename, std::string& out_full_path);
