#pragma once

#include "Macros.hpp"
#include "StringLiteral.hpp"

#include <array>
#include <iostream>
#include <source_location>
#include <string_view>

// Macros and functions for logging

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
#define ATT_DETAIL_COUT_HERE(p_logTag) \
    ::utils::detail::LogOStreamScoped(::utils::LogTag::p_logTag, __FILE__, __LINE__)

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_INFO
#    define ATT_LOG_ERROR_AT(p_filePath, p_line, ...) ATT_DETAIL_LOG_AT(Error, p_filePath, p_line, __VA_ARGS__)
#    define ATT_LOG_ERROR(...) ATT_DETAIL_LOG_HERE(Error, __VA_ARGS__)
#    define ATT_LOG_WARN(...) ATT_DETAIL_LOG_HERE(Warn, __VA_ARGS__)
#    define ATT_LOG_INFO(...) ATT_DETAIL_LOG(Info, __VA_ARGS__)
#    define ATT_COUT_INFO() ATT_DETAIL_COUT_HERE(Info)
#else
#    define ATT_LOG_LOC_ERROR(p_filePath, p_line, ...) ATT_NOOP()
#    define ATT_LOG_ERROR(...) ATT_NOOP()
#    define ATT_LOG_WARN(...) ATT_NOOP()
#    define ATT_LOG_INFO(...) ATT_NOOP()
#    define ATT_COUT_INFO() ::utils::detail::NoopOStream()
#endif

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_DEBUG
#    define ATT_LOG_DEBUG(...) ATT_DETAIL_LOG_HERE(Debug, __VA_ARGS__)
#else
#    define ATT_LOG_DEBUG(...) ATT_NOOP()
#endif

namespace utils
{

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

constexpr inline std::string_view LogTagToString(LogTag tag) noexcept
{
    constexpr std::array<std::string_view, 5> strs = {
        "ASSERT",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG"};
    return strs[static_cast<int>(tag)];
}

template <typename T>
inline void LogValue(const T& value)
{
    std::clog << value;
}
inline void LogValue(bool value)
{
    std::clog << (value ? "true" : "false");
}
inline void LogValue(std::string_view str)
{
    // add indentation to newlines
    auto begin = str.begin();
    for (auto it = begin; it != str.end(); ++it)
    {
        if (*it != '\n') continue;
        std::clog << std::string_view(begin, it) << "\n|     ";
        begin = it + 1;
    }
    std::clog << std::string_view(begin, str.end());
}
template <std::size_t N>
constexpr void LogValue(RawStringLiteral<N> str)
{
    if constexpr (N >= 1)
    {
        const std::size_t length = (str[N - 1] == '\0') ? N - 1 : N;
        LogValue(std::string_view(str, length));
    }
}

} // namespace detail

/// logs [WARN: M123456789 10.1234s]
void LogPrelude(LogTag tag) noexcept;
/// logs [WARN: M123456789 10.1234s] (/path/to/file.cpp:10)
void LogPrelude(LogTag tag, std::string_view filePath, int line) noexcept;

template <typename... TArgs>
inline void LogValues(const TArgs&... args) noexcept
{
    try
    {
        (detail::LogValue(args), ...);
    }
    catch (...)
    {
        // throw away logging errors
        // logging should be used in any context, including destructors
        // don't want to ruin control flow when not expected
    }
}

inline void LogEnd() noexcept
{
    try
    {
        std::clog << std::endl;
    }
    catch (...)
    {
        // see LogValues
    }
}

template <typename... TArgs>
struct Log
{
    constexpr explicit Log(const TArgs&... args, const std::source_location& loc = std::source_location::current()) noexcept
    {
        LogPrelude(LogTag::Info, loc.file_name(), static_cast<int>(loc.line()));
        LogValues(args...);
        LogEnd();
    }
};

template <typename... TArgs>
Log(const TArgs&...) -> Log<TArgs...>;

namespace detail
{

struct LogOStreamScoped
{
    LogOStreamScoped(LogTag tag, const char* filePath, int line) noexcept
    {
        LogPrelude(tag, filePath, line);
    }
    explicit LogOStreamScoped(LogTag tag) noexcept
    {
        LogPrelude(tag);
    }
    ~LogOStreamScoped()
    {
        LogEnd();
    }
    template <typename T>
    void operator<<(const T& value) const noexcept
    {
        LogValues(value);
    }
};

struct NoopOStream
{
    template <typename T>
    constexpr void operator<<(const T&) const noexcept {}
};

} // namespace detail

} // namespace utils
