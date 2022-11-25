#include "LogFileHandler.hpp"

#include "Env.hpp"
#include "Log.hpp"

#include <filesystem>

namespace utils
{

namespace fsys = std::filesystem;

void LogFileHandler::RedirectConsoleToFile()
{
    const fsys::path logsDir = GetLogsDir();
    if (!fsys::exists(logsDir)) fsys::create_directory(logsDir);
    const fsys::path latestLogPath = logsDir / GetLogFileName("latest");
    if (fsys::exists(latestLogPath))
    {
        // didn't call CloseAndTimestampFile() last exit, probably crashed
        TimestampFile();
    }
    mLogWriter.open(latestLogPath);
    if (!mLogWriter.is_open()) return;

    mCoutBuffer = std::cout.rdbuf();
    mCerrBuffer = std::cerr.rdbuf();
    mClogBuffer = std::clog.rdbuf();
    std::cout.rdbuf(mLogWriter.rdbuf());
    std::cerr.rdbuf(mLogWriter.rdbuf());
    std::clog.rdbuf(mLogWriter.rdbuf());
    ATT_LOG_INFO("redirected console to file");
}

void LogFileHandler::CloseAndTimestampFile()
{
    ATT_LOG_INFO("closing log file");
    std::cout.flush();
    std::cerr.flush();
    std::clog.flush();
    std::cout.rdbuf(mCoutBuffer);
    std::cerr.rdbuf(mCerrBuffer);
    std::clog.rdbuf(mClogBuffer);
    mLogWriter.close();
    TimestampFile();
}

void LogFileHandler::TimestampFile()
{
    const fsys::path latestLogPath = GetLogsDir() / GetLogFileName("latest");
    const fsys::path stampedLogPath = GetLogsDir() / GetLogFileName(GetTimestamp());
    if (fs::exists(latestLogPath) && !fs::exists(stampedLogPath))
    {
        fs::rename(latestLogPath, stampedLogPath);
    }
}

std::string LogFileHandler::GetLogFileName(std::string_view tag)
{
    std::string name = "out_";
    name += tag;
    name += ".log";
    return name;
}

std::string LogFileHandler::GetTimestamp()
{
    auto timestamp = GetAppStartTimePoint().time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timestamp);
    return std::to_string(seconds.count());
}

} // namespace utils
