#pragma once

#include "Macros.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

// Macros and functions for logging,
// The argument '\n' will newline and indent the log entry

// Log verbosity levels, choose what gets printed to stderr
/// No logging (except assertion fails)
#define ATT_LOG_LEVEL_SILENT 0
/// (default release) enable almost all logging macros
#define ATT_LOG_LEVEL_INFO 1
/// (default debug) enable ATT_LOG_DEBUG
#define ATT_LOG_LEVEL_DEBUG 2

/// log at a line of code with severity
#define ATT_DETAIL_LOG_AT(p_logTag, p_filePath, p_line, ...)                \
    static_cast<void>(                                                      \
        ::utils::LogPrelude(::utils::LogTag::p_logTag, p_filePath, p_line), \
        ::utils::LogValues(__VA_ARGS__),                                    \
        ::utils::LogEnd())
/// log at the current line of code with severity
#define ATT_DETAIL_LOG_HERE(p_logTag, ...) \
    ATT_DETAIL_LOG_AT(p_logTag, __FILE__, __LINE__, __VA_ARGS__)
/// log with severity
#define ATT_DETAIL_LOG(p_logTag, ...)                   \
    static_cast<void>(                                  \
        ::utils::LogPrelude(::utils::LogTag::p_logTag), \
        ::utils::LogValues(__VA_ARGS__),                \
        ::utils::LogEnd())

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_INFO
#    define ATT_LOG_ERROR_AT(p_filePath, p_line, ...) ATT_DETAIL_LOG_AT(Error, p_filePath, p_line, __VA_ARGS__)
#    define ATT_LOG_ERROR(...) ATT_DETAIL_LOG_HERE(Error, __VA_ARGS__)
#    define ATT_LOG_WARN(...) ATT_DETAIL_LOG_HERE(Warn, __VA_ARGS__)
#    define ATT_LOG_INFO(...) ATT_DETAIL_LOG(Info, __VA_ARGS__)
#else
#    define ATT_LOG_LOC_ERROR(p_filePath, p_line, ...) ATT_NOOP()
#    define ATT_LOG_ERROR(...) ATT_NOOP()
#    define ATT_LOG_WARN(...) ATT_NOOP()
#    define ATT_LOG_INFO(...) ATT_NOOP()
#endif

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_DEBUG
#    define ATT_LOG_DEBUG(...) ATT_DETAIL_LOG_HERE(Debug, __VA_ARGS__)
#else
#    define ATT_LOG_DEBUG(...) ATT_NOOP()
#endif

namespace utils
{
namespace fs = std::filesystem;

enum class LogTag
{
    Assert,
    Error,
    Warn,
    Info,
    Debug,
};

namespace detail
{

constexpr inline std::string_view LogTagToString(LogTag tag)
{
    constexpr std::array<std::string_view, 5> strs = {
        "ASSERT",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG"};
    return strs.at(static_cast<int>(tag));
}

template <typename T>
inline void LogValue(const T& value)
{
    std::clog << value;
}
inline void LogValue(char value)
{
    std::clog << value;
    if (value == '\n') std::clog << "|     ";
}

} // namespace detail

/// logs [WARN: M123456789 10.1234s]
void LogPrelude(LogTag tag);
/// logs [WARN: M123456789 10.1234s] (/path/to/file.cpp:10)
void LogPrelude(LogTag tag, const char* filePath, int line);

template <typename... Ts>
inline void LogValues(Ts&&... vals)
{
    (detail::LogValue(std::forward<Ts>(vals)), ...);
}

/// logs newline
void LogEnd();

class LogFileHandler
{
public:
    LogFileHandler();

    void RedirectConsoleToFile();
    void CloseAndTimestampFile();

private:
    void TimestampFile();
    fs::path GetLogFilePath(std::string_view tag) const;
    static std::string GetTimestamp();

    fs::path logsDir{};
    std::ofstream logWriter{};
    std::streambuf* coutBuffer = nullptr;
    std::streambuf* cerrBuffer = nullptr;
    std::streambuf* clogBuffer = nullptr;
};

} // namespace utils
