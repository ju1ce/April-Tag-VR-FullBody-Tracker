#pragma once

#include <cstdlib>
#include <exception>

#if defined(DEBUG) || defined(_DEBUG) || !defined(DNDEBUG)

#define ATT_ENABLE_ASSERT

#ifndef ATT_LOG_LEVEL
#define ATT_LOG_LEVEL 2
#endif

#else

#ifndef ATT_LOG_LEVEL
#define ATT_LOG_LEVEL 0
#endif

#endif

#if ATT_LOG_LEVEL > 0
#include <chrono>
#include <iostream>
#include <sstream>
#endif

// Macros and functions for debugging and logging,

// Prefix all macros with ATT_ or AT if needs to be shortened
// and prioritize defining only a small amount of simple macros

// Log verbosity levels, choose what gets printed to stderr,
//  enables all log levels from 0 to ATT_LOG_LEVEL
// 0 - NOTHING: nothing will be printed to stderr.
// 1 - FATAL: Unrecoverable state, followed by abort()
// 2 - THROW: Recoverable state, when c++ exception is thrown
// 3 - TRACE: Fine grained status.

// Input Defines:
//  ATT_ENABLE_ASSERT
//  ATT_LOG_LEVEL
// Public Macros:
//  ATASSERT(messageStream, condition)
//  ATFATAL(messageStream)
//  ATTHROW(exceptionType, messageStream)
//  ATTRACE(messageStream)
// Internal:
//  ATT_LOG(messageStream)
//  ATT_STRINGIZE_ARG(arg)
//  ATT_PRETTY_FUNCTION

/// Necessary to expand the __LINE__ macro into a string
#define ATT_STRINGIZE_ARG_(arg) #arg
#define ATT_STRINGIZE_ARG(arg) ATT_STRINGIZE_ARG_(arg)

#if defined(__clang__) || defined(__GNUG__) || defined(__GNUC__)
#define ATT_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define ATT_PRETTY_FUNCTION __FUNCSIG__
#else
#define ATT_PRETTY_FUNCTION ""
#endif

#if ATT_LOG_LEVEL > 0
#define ATT_LOG(a_messageStream)                                                            \
    std::cerr << "(" << std::chrono::system_clock::now().time_since_epoch().count() << ") " \
              << "[" << __FILE__ << ":" ATT_STRINGIZE_ARG(__LINE__) "] "                    \
              << a_messageStream                                                            \
              << std::endl
#else
#define ATT_LOG(a_messageStream) ((void)0)
#endif

#if ATT_LOG_LEVEL >= 1
#define ATFATAL(a_messageStream)                     \
    do {                                             \
        ATT_LOG("Fatal Error: " << a_messageStream); \
        Debug::abort();                              \
    } while (0)
#else
#define ATFATAL(a_messageStream) Debug::abort()
#endif

#if ATT_LOG_LEVEL >= 2
#define ATTHROW(a_exceptionType, a_messageStream)               \
    do {                                                        \
        std::stringstream msgStream;                            \
        msgStream << a_messageStream;                           \
        ATT_LOG(#a_exceptionType << ": " << msgStream); \
        throw a_exceptionType(msg_stream.str());                \
    } while (0)
#else
#define ATTHROW(a_exception_type, a_messageStream)       \
    do {                                                 \
        std::stringstream msgStream << a_messageStream; \
        throw a_exceptionType(msgStream.str())         \
    } while (0)
#endif

#if ATT_LOG_LEVEL >= 3
#define ATTRACE(a_messageStream) \
    ATT_LOG("Trace: " << a_messageStream)
#else
#define ATTRACE(a_messageStream) ((void)0)
#endif

#ifdef ATT_ENABLE_ASSERT
#define ATASSERT(a_messageStream, a_expression)                                             \
    do {                                                                                    \
        if (!(a_expression))                                                                \
            ATFATAL(a_messageStream << std::endl                                            \
                                    << "    Assertion failure: ( " << #a_expression << " )" \
                                    << " in " << ATT_PRETTY_FUNCTION);                      \
    } while (0)
#else
#define ATASSERT(a_messageStream, a_expression) ((void)0)
#endif

namespace Debug
{
/// Intercept abort calls for easier debugging
/// Named the same so that 'abort' breakpoints will find it
inline void abort()
{
    std::abort();
}

}
