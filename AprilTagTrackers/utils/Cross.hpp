#pragma once

#ifdef ATT_DEBUG
#    include "Test.hpp" // IWYU pragma: keep
#else
#    include <cstdlib> // IWYU pragma: keep
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

#if defined(ATT_COMP_CLANG) || defined(ATT_COMP_GCC)
#    define ATT_DETAIL_UNREACHABLE() __builtin_unreachable()
#elif defined(ATT_COMP_MSVC)
#    define ATT_DETAIL_UNREACHABLE() __assume(false)
#else
#    define ATT_DETAIL_UNREACHABLE() \
        while (true)                 \
            ;
#endif

#ifdef ATT_DEBUG
#    define ATT_UNREACHABLE()         \
        do {                          \
            ATT_ABORT();              \
            ATT_DETAIL_UNREACHABLE(); \
        } while (false)
#else
#    define ATT_UNREACHABLE()         \
        do {                          \
            ATT_DETAIL_UNREACHABLE(); \
        } while (false)
#endif
