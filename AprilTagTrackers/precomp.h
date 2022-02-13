#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__CYGWIN__) || defined(_MSC_VER)
    #define OS_WIN 1
#elif defined(unix) || defined(__unix__)
    #define OS_LINUX 1
#elif defined(__APPLE__)
    #define OS_MACOS 1
    #error Building on MacOS is not currently supported
#else
    #error Unknown OS
#endif

#if OS_WIN
    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#else
    #include <errno.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <sys/un.h>
    #include <unistd.h>
#endif

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include <unordered_map>

#include <cmath>
#include <random>
#include <chrono>

#include <thread>
#include <mutex>

#include <memory>

#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/registry.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>

#include <openvr.h>

#pragma warning(push)
#pragma warning(disable:4996)
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/string.h>
#pragma warning(pop)