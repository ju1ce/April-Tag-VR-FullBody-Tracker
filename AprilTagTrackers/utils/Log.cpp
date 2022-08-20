#include "Log.hpp"

#include "Env.hpp"

#include <iomanip>
#include <string>

namespace utils
{

void LogPrelude(LogTag tag)
{
    std::cerr << std::boolalpha
              << "["
              << detail::LogTagToString(tag) << ": "
              << (IsMainThread() ? 'M' : ' ')
              << GetThisThreadID() << ' '
              << std::fixed << std::setprecision(4)
              << GetRuntimeSeconds()
              << std::defaultfloat
              << "s] ";
}

void LogPrelude(LogTag tag, const char* filePath, int line)
{
    LogPrelude(tag);
    if (filePath != nullptr)
    {
        std::cerr << "(" << filePath;
        if (line >= 1) std::cerr << ":" << line;
        std::cerr << ") ";
    }
}

void LogEnd()
{
    std::cerr << '\n'
              << std::noboolalpha;
}

void LogFileHandler::RedirectConsoleToFile()
{
    logsDir = GetLogsDir();
    if (!fs::exists(logsDir))
        fs::create_directory(logsDir);
    fs::path latestLogPath = GetLogFilePath("latest");
    if (fs::exists(latestLogPath))
    {
        // didn't call CloseAndTimestampFile() last exit, probably crashed
        TimestampFile();
    }
    logWriter.open(latestLogPath);
    if (!logWriter.is_open()) return;

    coutBuffer = std::cout.rdbuf();
    cerrBuffer = std::cerr.rdbuf();
    std::cout.rdbuf(logWriter.rdbuf());
    std::cerr.rdbuf(logWriter.rdbuf());
    ATT_LOG_INFO("redirected console to file");
}

void LogFileHandler::CloseAndTimestampFile()
{
    ATT_LOG_INFO("closing log file");
    std::cout.rdbuf(coutBuffer);
    std::cerr.rdbuf(cerrBuffer);
    logWriter.close();
    TimestampFile();
}

void LogFileHandler::TimestampFile()
{
    fs::path latestLogPath = GetLogFilePath("latest");
    fs::path stampedLogPath = GetLogFilePath(GetTimestamp());
    if (fs::exists(latestLogPath) && !fs::exists(stampedLogPath))
    {
        fs::rename(latestLogPath, stampedLogPath);
    }
}

fs::path LogFileHandler::GetLogFilePath(std::string_view tag) const
{
    fs::path file = logsDir;
    file /= "out_";
    file += tag;
    file += ".log";
    return file;
}

std::string LogFileHandler::GetTimestamp()
{
    auto timestamp = GetAppStartTimePoint().time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timestamp);
    return std::to_string(seconds.count());
}

} // namespace utils
