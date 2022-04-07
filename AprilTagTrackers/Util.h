#pragma once

#include <chrono>
#include <thread>
#include <string>

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
    #define OS_WIN true
#elif defined(unix) || defined(__unix__)
    #define OS_LINUX true
#elif defined(__APPLE__)
    #define OS_MACOS true
    #error Building on MacOS is not currently supported
#else
    #error Unknown OS
#endif

void sleep_millis(int);
bool get_full_path(const std::string& filename, std::string& out_full_path);
