#pragma once

#include "Macros.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

// Macros and functions for logging,
// Prefix all macros with ATT_ and private macros with ATT_DETAILS_

// Log verbosity levels, choose what gets printed to stderr
/// No logging
#define ATT_LOG_LEVEL_SILENT 0
/// (default release) enable logging
#define ATT_LOG_LEVEL_INFO 2
/// (default debug) enable ATT_DEBUG
#define ATT_LOG_LEVEL_DEBUG 3

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_INFO
/// log at a Line Of Code with severity
#    define ATT_DETAILS_LOG_LOC(p_logTag, p_filePath, p_line, ...)           \
        (::utils::LogPrelude(::utils::LogTag::p_logTag, p_filePath, p_line), \
            ::utils::LogValues(__VA_ARGS__),                                 \
            ::utils::LogEnd())
/// log at the current Line Of Code with severity
#    define ATT_DETAILS_LOG_AT(p_logTag, ...)                                \
        (::utils::LogPrelude(::utils::LogTag::p_logTag, __FILE__, __LINE__), \
            ::utils::LogValues(__VA_ARGS__),                                 \
            ::utils::LogEnd())
/// log with severity
#    define ATT_DETAILS_LOG(p_logTag, ...)               \
        (::utils::LogPrelude(::utils::LogTag::p_logTag), \
            ::utils::LogValues(__VA_ARGS__),             \
            ::utils::LogEnd())

#    define ATT_LOG_ERROR(...) ATT_DETAILS_LOG_AT(Error, __VA_ARGS__)
#    define ATT_LOG_WARN(...) ATT_DETAILS_LOG_AT(Warn, __VA_ARGS__)
#    define ATT_LOG_INFO(...) ATT_DETAILS_LOG(Info, __VA_ARGS__)
#else
#    define ATT_LOG_ERROR(...) ATT_NOOP()
#    define ATT_LOG_WARN(...) ATT_NOOP()
#    define ATT_LOG_INFO(...) ATT_NOOP()
#endif

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_DEBUG
#    define ATT_LOG_DEBUG(...) ATT_DETAILS_LOG_AT(Debug, __VA_ARGS__)
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

namespace details
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
inline void LogValue(T&& value)
{
    std::cerr << value;
}
inline void LogValue(char value)
{
    std::cerr << value;
    if (value == '\n') std::cerr << "|     ";
}

} // namespace details

/// logs [WARN: M123456789 10.1234s]
void LogPrelude(LogTag tag);
/// logs [WARN: M123456789 10.1234s] (/path/to/file.cpp:10)
void LogPrelude(LogTag tag, const char* filePath, int line);

template <typename... Ts>
inline void LogValues(Ts&&... vals)
{
    (details::LogValue(std::forward<Ts>(vals)), ...);
}

/// logs newline
void LogEnd();

class LogFileHandler
{
public:
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
};

} // namespace utils
