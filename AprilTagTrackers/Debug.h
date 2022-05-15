#pragma once

#include <cstdlib>
#include <thread>

#if ATT_LOG_LEVEL > 0
#include <chrono>
#include <iomanip>
#include <iostream>
#endif

// Macros and functions for debugging and logging,

// Prefix all macros with ATT_ or AT if needs to be shortened
// and prioritize defining only a small amount of simple macros

// Log verbosity levels, choose what gets printed to stderr,
// enables all log levels from 0 to ATT_LOG_LEVEL
// 0 - NOTHING: nothing will be printed to stderr.
// 1 - FATAL: Unrecoverable state, followed by abort()
// 2 - ERROR: Recoverable state, when c++ exception is caught
// 3 - TRACE: Fine grained status.

// Input Defines:
//  ATT_ENABLE_ASSERT
//  ATT_LOG_LEVEL

// Public Macros:
//  Require an expression to be true
//   ATASSERT(messageStream, condition)

//  Require an expression to be true, like assert,
//  but the expression can have side effects even when assert is disabled
//   ATCHECK(messageStream, trueCondition)

//  Logs a fatal error, when LOG_LEVEL is 1.
//  Calls abort().
//   ATFATAL(messageStream)

//  Logs an error, when LOG_LEVEL is 2
//   ATERROR(messageStream)

//  Logs a trace, when LOG_LEVEL is 3
//   ATTRACE(messageStream)

// Internal:
//  messageStream is a sequence of strings and insertion operators, pasted into a std::cerr statement
//   ATT_LOG(messageStream)

//  Prints the function the macro is used in
//   ATT_PRETTY_FUNCTION

#if defined(__clang__) || defined(__GNUG__) || defined(__GNUC__)
/// Prints the function the macro is used in
#define ATT_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
/// Prints the function the macro is used in
#define ATT_PRETTY_FUNCTION __FUNCSIG__
#else
/// Prints the function the macro is used in (fallback)
#define ATT_PRETTY_FUNCTION ""
#endif

#if ATT_LOG_LEVEL > 0
/// messageStream is a sequence of strings and insertion operators, pasted into a std::cerr statement
#define ATT_LOG(a_messageStream)       \
    Debug::PreLog(__FILE__, __LINE__); \
    std::cerr << a_messageStream << std::endl
#else
/// messageStream is a sequence of strings and insertion operators, pasted into a std::cerr statement
#define ATT_LOG(a_messageStream) \
    static_cast<void>(0)
#endif

#if ATT_LOG_LEVEL >= 1
/// Logs a fatal error, when LOG_LEVEL is 1.
/// Calls abort().
#define ATFATAL(a_messageStream)                     \
    do {                                             \
        ATT_LOG("Fatal Error: " << a_messageStream); \
        Debug::abort();                              \
    } while (0)
#else
/// Logs a fatal error, when LOG_LEVEL is 1.
/// Calls abort().
#define ATFATAL(a_messageStream) \
    Debug::abort()
#endif

#if ATT_LOG_LEVEL >= 2
/// Logs an error, when LOG_LEVEL is 2
#define ATERROR(a_messageStream) \
    ATT_LOG("Error: " << a_messageStream);
#else
/// Logs an error, when LOG_LEVEL is 2
#define ATERROR(a_messageStream) \
    static_cast<void>(0)
#endif

#if ATT_LOG_LEVEL >= 3
/// Logs a trace, when LOG_LEVEL is 3
#define ATTRACE(a_messageStream) \
    ATT_LOG("Trace: " << a_messageStream)
#else
/// Logs a trace, when LOG_LEVEL is 3
#define ATTRACE(a_messageStream) \
    static_cast<void>(0)
#endif

#ifdef ATT_ENABLE_ASSERT

/// Require an expression to be true
#define ATASSERT(a_messageStream, a_trueExpression) \
    do {                                            \
        if (!(a_trueExpression))                    \
            ATFATAL("Assert: "                      \
                    << a_messageStream              \
                    << std::endl                    \
                    << "    Assertion failure:  ( " \
                    << #a_trueExpression            \
                    << " )  in  "                   \
                    << ATT_PRETTY_FUNCTION);        \
    } while (0)

/// Require an expression to be true, like assert,
/// but the expression can have side effects even when assert is disabled
#define ATCHECK(a_messageStream, a_trueExpression) \
    ATASSERT(a_messageStream, a_trueExpression)

#else

/// Require an expression to be true
#define ATASSERT(a_messageStream, a_trueExpression) \
    static_cast<void>(0)

/// Require an expression to be true, like assert,
/// but the expression can have side effects even when assert is disabled
#define ATCHECK(a_messageStream, a_trueExpression) \
    static_cast<void>(a_trueExpression)

#endif

namespace Debug
{
/// this_thread::get_id called during static initialization
extern const std::thread::id mainThreadID;

/// Test if calling thread is the main thread, (whatever static initialization set)
inline bool IsMainThread()
{
    return std::this_thread::get_id() == mainThreadID;
}

/// Intercept abort calls for easier debugging
/// Named the same so that 'abort' breakpoints will find it
inline void abort()
{
    // https://stackoverflow.com/a/4326567/18158409
    // NOLINTBEGIN
#if defined(WIN32)
    __debugbreak();
    std::exit(3);
#elif defined(__APPLE__)
    *((int*)nullptr) = 0;
    std::abort();
#else
    std::abort();
#endif
    // NOLINTEND
}

#if ATT_LOG_LEVEL > 0
/// system_clock::now() called at static initialization
extern const std::chrono::system_clock::time_point appStartTimePoint;

/// [ thread_id @ runtime_sec ] (file:line)
template <typename StrT>
inline void PreLog(const StrT file, int line) noexcept
{
    const auto stamp = std::chrono::system_clock::now() - appStartTimePoint;
    const auto stampSec = std::chrono::duration<float>(stamp).count();

    std::cerr << "[ "
              << (Debug::IsMainThread() ? "M" : "")
              << std::this_thread::get_id() << " @ "
              << std::fixed << std::setprecision(4)
              << stampSec << std::defaultfloat // reset std::fixed
              << " ] (" << file
              << ":" << line << ") ";
}

#endif

} // namespace Debug
