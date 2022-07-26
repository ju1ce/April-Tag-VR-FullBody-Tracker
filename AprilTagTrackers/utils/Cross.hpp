#pragma once

#ifdef ATT_DEBUG
#    include <doctest/doctest.h> // IWYU pragma: keep
#else
#    include <cstdlib> // IWYU pragma: keep
#endif

// compiler detection
#if defined(__clang__)
#    define ATT_COMP_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
#    define ATT_COMP_GCC 1
#elif defined(_MSC_VER)
#    define ATT_COMP_MSVC 1
#else
#    define ATT_COMP_UNKNOWN 1
#endif

// os detection
#if defined(_WIN32) || defined(__WINDOWS__)
#    define ATT_OS_WINDOWS 1
#elif defined(__APPLE__) || defined(__MACH__)
#    define ATT_OS_MACOS 1
#elif defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#    define ATT_OS_LINUX 1
#else
#    define ATT_OS_UNKNOWN 1
#endif

// function signature
#if defined(ATT_COMP_CLANG) || defined(ATT_COMP_GCC)
#    define ATT_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(ATT_COMP_MSVC)
#    define ATT_PRETTY_FUNCTION __FUNCSIG__
#else
#    define ATT_PRETTY_FUNCTION "<unknown function>()"
#endif

// debug break or abort
#ifdef ATT_DEBUG
#    define ATT_ABORT() [] {           \
        DOCTEST_BREAK_INTO_DEBUGGER(); \
    }()
#else
#    define ATT_ABORT() std::abort()
#endif

#ifdef ATT_DEBUG
#    define ATT_UNREACHABLE() ATT_ABORT()
#else
#    if defined(ATT_COMP_CLANG) || defined(ATT_COMP_GCC)
#        define ATT_UNREACHABLE() __builtin_unreachable()
#    elif defined(ATT_COMP_MSVC)
#        define ATT_UNREACHABLE() __assume(false)
#    else
#        define ATT_UNREACHABLE() ATT_ABORT()
#    endif
#endif
