#pragma once

#include "utils/Cross.hpp"
#include "utils/Macros.hpp"

#ifdef ATT_TESTING
#include <doctest/doctest.h>
#endif

#include <array>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <thread>

// Macros and functions for logging,
// Prefix all macros with ATT_ and private macros with ATT_DETAILS_

// Log verbosity levels, choose what gets printed to stderr,
// enables all log levels from 0 to ATT_LOG_LEVEL
/// No logging
#define ATT_LOG_LEVEL_SILENT 0
/// logs: ATT_ASSERT, ATT_ERROR
#define ATT_LOG_LEVEL_ERROR 1
/// (default release) logs: ATT_WARN, ATT_INFO
#define ATT_LOG_LEVEL_INFO 2
/// (default debug) logs: previous, ATT_DEBUG
#define ATT_LOG_LEVEL_DEBUG 3

#ifdef ATT_TESTING
#define ATT_DETAILS_LOG(p_logLevel, ...) \
    ::utils::details::TestLog(::utils::LogTag::p_logLevel, __FILE__, __LINE__, __VA_ARGS__)
#define ATT_DETAILS_LOG_LIB(p_logLevel, p_file, p_line, ...) \
    ::utils::details::TestLog(::utils::LogTag::p_logLevel, p_file, p_line, __VA_ARGS__)
#else
#define ATT_DETAILS_LOG(p_logLevel, ...) \
    ::utils::Log(::utils::LogTag::p_logLevel, __FILE__, __LINE__, __VA_ARGS__)
#define ATT_DETAILS_LOG_LIB(p_logLevel, p_file, p_line, ...) \
    ::utils::LogAbsolute(::utils::LogTag::p_logLevel, p_file, p_line, __VA_ARGS__)
#endif

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_ERROR
#define ATT_LOG_ERROR(...) ATT_DETAILS_LOG(Error, __VA_ARGS__)
#define ATT_LOG_LIB_ERROR(p_file, p_line, ...) ATT_DETAILS_LOG_LIB(Error, p_file, p_line, __VA_ARGS__)
#define ATT_DETAILS_ASSERT(p_expr, ...) ATT_DETAILS_LOG(Assert, ##__VA_ARGS__)
#else
#define ATT_LOG_ERROR(...) ATT_NOOP()
#define ATT_LOG_LIB_ERROR(p_file, p_line, ...) ATT_NOOP()
#define ATT_DETAILS_ASSERT(...) ATT_NOOP()
#endif

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_INFO
#define ATT_LOG_WARN(...) ATT_DETAILS_LOG(Warn, __VA_ARGS__)
#define ATT_LOG_INFO(...) ATT_DETAILS_LOG(Info, __VA_ARGS__)
#else
#define ATT_LOG_WARN(...) ATT_NOOP()
#define ATT_LOG_INFO(...) ATT_NOOP()
#endif

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_DEBUG
#define ATT_LOG_DEBUG(...) ATT_DETAILS_LOG(Debug, __VA_ARGS__)
#else
#define ATT_LOG_DEBUG(...) ATT_NOOP()
#endif

namespace utils
{
using system_clock = std::chrono::system_clock;
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
enum class LogAction
{
    None,
    TestFail,
    Abort
};
struct LogTagInfo
{
    int level;
    std::string_view name;
    std::ostream& os;
    LogAction action;
};

constexpr inline LogTagInfo GetLogTagInfo(LogTag tag)
{
    constexpr std::array infos = {
        LogTagInfo{ATT_LOG_LEVEL_ERROR, "ASSERT", std::cerr, LogAction::Abort},
        LogTagInfo{ATT_LOG_LEVEL_ERROR, "ERROR", std::cerr, LogAction::TestFail},
        LogTagInfo{ATT_LOG_LEVEL_INFO, "WARN", std::cerr, LogAction::TestFail},
        LogTagInfo{ATT_LOG_LEVEL_INFO, "INFO", std::cout, LogAction::None},
        LogTagInfo{ATT_LOG_LEVEL_DEBUG, "DEBUG", std::cout, LogAction::None}};
    return infos.at(static_cast<int>(tag));
}

/// this_thread::get_id called during static initialization
inline const std::thread::id MAIN_THREAD_ID = std::this_thread::get_id();
thread_local inline const std::thread::id THIS_THREAD_ID = std::this_thread::get_id();
thread_local inline const bool THIS_IS_MAIN_THREAD = MAIN_THREAD_ID == THIS_THREAD_ID;
/// time point of application startup
inline const system_clock::time_point APP_START_TP = system_clock::now();
/// used to find relative path of source files
inline const fs::path ATT_SOURCES_ROOT = fs::path(__FILE__)  // repo/AprilTagTrackers/utils/Log.hpp
                                             .parent_path()  // repo/AprilTagTrackers/utils/
                                             .parent_path()  // repo/AprilTagTrackers/
                                             .parent_path(); // repo/
} // namespace details

inline bool IsMainThread()
{
    return details::THIS_IS_MAIN_THREAD;
}

inline double RuntimeSeconds()
{
    return std::chrono::duration<double>(system_clock::now() - details::APP_START_TP).count();
}

inline std::string RelativeSourcePath(std::string_view filePath)
{
    return fs::path(filePath).lexically_relative(details::ATT_SOURCES_ROOT).generic_u8string();
}

namespace details
{

/// [WARN: M123456789 @10.1234s] (AprilTagTrackers/Tracker.cpp:12)
inline void PreLog(std::ostream& os, std::string_view tag, std::string_view filePath, int line)
{
    os << "[";
    if (!tag.empty()) os << tag << ": ";
    if (THIS_IS_MAIN_THREAD) os << "M";
    os << THIS_THREAD_ID << " @"
       << std::fixed << std::setprecision(4)
       << RuntimeSeconds() << std::defaultfloat
       << "s] ";
    if (!filePath.empty())
    {
        os << "(" << filePath;
        if (line >= 1) os << ":" << line;
        os << ") ";
    }
}
template <typename T>
inline void Log(std::ostream& os, const T& value)
{
    os << value;
}
template <>
inline void Log(std::ostream& os, const char& value)
{
    os << value;
    if (value == '\n') os << "|     ";
}

#ifdef ATT_TESTING
template <typename... Ts>
inline void TestLog(LogTag tag, const char* const file, int line, const Ts&... vals)
{
    const auto& level = details::GetLogTagInfo(tag);
    if (level.action == LogAction::Abort)
    {
        DOCTEST_ADD_FAIL_AT(file, line, level.name);
    }
    else if (level.action == LogAction::TestFail)
    {
        DOCTEST_ADD_FAIL_CHECK_AT(file, line, level.name);
    }
    else
    {
        DOCTEST_ADD_MESSAGE_AT(file, line, level.name);
    }
    (details::Log(level.os, vals), ...);
}
#endif

} // namespace details

/// doesn't transform filepath relative to AprilTagTrackers, as library sources could be anywhere.
template <typename... Ts>
inline void LogAbsolute(LogTag tag, std::string_view filePath, int line, const Ts&... vals)
{
    const auto& level = details::GetLogTagInfo(tag);
    details::PreLog(level.os, level.name, filePath, line);
    (details::Log(level.os, vals), ...);
}

template <typename... Ts>
inline void Log(LogTag tag, std::string_view filePath, int line, const Ts&... vals)
{
    LogAbsolute(tag, RelativeSourcePath(filePath), line, vals...);
}

} // namespace utils
