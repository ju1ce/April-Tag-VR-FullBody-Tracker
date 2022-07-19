#include "MyApp.hpp"

#include <opencv2/core/utils/logger.hpp>

#ifdef ATT_DEBUG
#include <exception>
#include <stdexcept>
#endif

#ifdef ATT_LOG_TO_FILE
#include <fstream>
#include <iostream>
#endif

wxIMPLEMENT_APP(MyApp);

int MyApp::OnExit()
{
    tracker->Stop();
    return 0;
}

bool MyApp::OnInit()
{
    userConfig.Load();
    calibConfig.Load();
    arucoConfig.Load();
    lc.LoadLang(userConfig.langCode);

    tracker = std::make_unique<Tracker>(userConfig, calibConfig, arucoConfig, lc);
    gui = std::make_unique<GUI>(tracker, lc, userConfig);

    return true;
}

#ifdef ATT_LOG_TO_FILE

static std::ofstream outputLogFileWriter{"output.log"};

static const bool consoleOutputRedirected = ([]()
    {
        std::cout.rdbuf(outputLogFileWriter.rdbuf());
        std::cerr.rdbuf(outputLogFileWriter.rdbuf());
        return true;
    })();

#endif

#ifndef ATT_DEBUG

// Expand in place to maintain stack frames
#define HANDLE_UNHANDLED_EXCEPTION(a_msgContext)          \
    const auto ePtr = std::current_exception();           \
    try                                                   \
    {                                                     \
        RethrowStoredException();                         \
        if (ePtr) std::rethrow_exception(ePtr);           \
    }                                                     \
    catch (const std::exception& e)                       \
    {                                                     \
        ATFATAL(a_msgContext << ": " << e.what());        \
    }                                                     \
    catch (...)                                           \
    {                                                     \
        ATFATAL(a_msgContext << ": malformed exception"); \
    }                                                     \
    ATFATAL(a_msgContext << ": unknown exception");

void MyApp::OnFatalException()
{
    HANDLE_UNHANDLED_EXCEPTION("wxApp::OnFatalException");
}
void MyApp::OnUnhandledException()
{
    HANDLE_UNHANDLED_EXCEPTION("wxApp::OnUnhandledException");
}
bool MyApp::OnExceptionInMainLoop()
{
    HANDLE_UNHANDLED_EXCEPTION("wxApp::OnExceptionInMainLoop");
    return true; // suppress warning
}

// cv::ErrorCallback
static int OpenCVErrorHandler(int status, const char* funcName, const char* errMsg, const char* fileName, int line, void*)
{
    ATT_LOG_LIB_ERROR(fileName, line, "OpenCV Error(", status, "): ", errMsg, '\n', "in  ", funcName);
    ATT_ABORT();
    ATT_UNREACHABLE();
}

// wxAssertHandler_t
static void wxWidgetsAssertHandler(const wxString& file, int line, const wxString& func, const wxString& cond, const wxString& msg)
{
    ATT_LOG_LIB_ERROR(file.utf8_string(), line, "wxWidgets Error: ", msg, '\n',
        "assertion failure  ( ", cond.utf8_string(), " )  in  ", func);
    ATT_ABORT();
}

static const bool errorHandlersRedirected = ([]()
    {
        cv::redirectError(&OpenCVErrorHandler);
        wxSetAssertHandler(&wxWidgetsAssertHandler);
        return true;
    })();

#endif

static const bool opencvSetLogLevelWarning = ([]()
    {
        cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
        return true;
    })();
