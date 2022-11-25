#pragma once

#include <fstream>
#include <streambuf>
#include <string>


namespace utils
{

class LogFileHandler
{
public:
    void RedirectConsoleToFile();
    void CloseAndTimestampFile();

private:
    static void TimestampFile();
    static std::string GetLogFileName(std::string_view tag);
    static std::string GetTimestamp();

    std::ofstream mLogWriter{};
    std::streambuf* mCoutBuffer = nullptr;
    std::streambuf* mCerrBuffer = nullptr;
    std::streambuf* mClogBuffer = nullptr;
};

} // namespace utils
