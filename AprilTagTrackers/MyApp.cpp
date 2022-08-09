#include "MyApp.hpp"

#include "utils/Assert.hpp"
#include "utils/Log.hpp"

#include <opencv2/core/utils/logger.hpp>

wxIMPLEMENT_APP(MyApp); // NOLINT

int MyApp::OnExit()
{
    tracker->Stop();

    if (envVars.IsRedirectConsoleToFile())
        logFileHandler.CloseAndTimestampFile();
    return 0;
}

bool MyApp::OnInit()
{
    // OnAssertFailure(const wxChar* file, int line, const wxChar* func, const wxChar* cond, const wxChar* msg);
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_INFO);

    if (envVars.IsRedirectConsoleToFile())
        logFileHandler.RedirectConsoleToFile();
    ATT_LOG_INFO("Starting AprilTagTrackers");

    userConfig.Load();

    // The next two lines were added as a quick fix. The two options should be handeled differently from other parameters, so as a quick fix, they are reset on every launch of ATT.
    // a quick patch to prevent parameters from having an outdated version number
    userConfig.driver_version = SemVer::Parse(ATT_STRINGIZE(ATT_DRIVER_VERSION));
    // since disable openvr api isnt part of the parameters, it isnt loaded properly. This ensures it is globaly disabled on every launch.
    userConfig.disableOpenVrApi = false;

    calibConfig.Load();
    arucoConfig.Load();
    lc.LoadLang(userConfig.langCode);

    tracker = std::make_unique<Tracker>(userConfig, calibConfig, arucoConfig, lc);
    gui = std::make_unique<GUI>(tracker, lc, userConfig);

    return true;
}

#ifdef ATT_DEBUG

void MyApp::OnFatalException()
{
    ATT_FATAL_EXCEPTION(RethrowStoredException(), "wxApp::OnFatalException");
}
void MyApp::OnUnhandledException()
{
    ATT_FATAL_EXCEPTION(RethrowStoredException(), "wxApp::OnUnhandledException");
}
bool MyApp::OnExceptionInMainLoop()
{
    ATT_FATAL_EXCEPTION(RethrowStoredException(), "wxApp::OnExceptionInMainLoop");
    return true;
}

// cv::ErrorCallback
static int OpenCVErrorHandler(int status, const char* funcName, const char* errMsg, const char* fileName, int line, void*)
{
    ATT_LOG_LOC_ERROR(fileName, line, "OpenCV Error(", status, "): ", errMsg, "\nin  ", funcName);
    ATT_ABORT();
    return 0;
}

// wxAssertHandler_t
static void wxWidgetsAssertHandler(const wxString& file, int line, const wxString& func, const wxString& cond, const wxString& msg)
{
    ATT_LOG_LOC_ERROR(file.c_str().AsChar(), line, "wxWidgets Error: ", msg,
        "\nassertion failure  ( ", cond, " )  in  ", func);
    ATT_ABORT();
}

static inline const bool overrideErrorHandlers = []
{
    cv::redirectError(&OpenCVErrorHandler);
    wxSetAssertHandler(&wxWidgetsAssertHandler);
    return true;
}();

#endif
