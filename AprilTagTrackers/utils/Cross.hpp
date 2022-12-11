#pragma once

#ifdef ATT_DEBUG
#    include "Test.hpp" // IWYU pragma: keep
#endif

#include <cstdlib> // IWYU pragma: keep

// function signature
#if defined(ATT_COMP_CLANG) || defined(ATT_COMP_GCC)
#    define ATT_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(ATT_COMP_MSVC)
#    define ATT_PRETTY_FUNCTION __FUNCSIG__
#else
#    define ATT_PRETTY_FUNCTION "<unknown function>()"
#endif

// trigger a debugger breakpoint
// on linux this is a statement, and some debuggers cannot resume from it
#define ATT_DEBUG_BREAK() DOCTEST_BREAK_INTO_DEBUGGER()

// wrap ATT_DEBUG_BREAK in lambda on linux to make it an expression
// adds an extra lambda to the callstack, but the location will be correct
#ifdef ATT_COMP_MSVC
#    define ATT_DEBUG_BREAK_EXPR() ATT_DEBUG_BREAK()
#else
#    define ATT_DEBUG_BREAK_EXPR() [] { ATT_DEBUG_BREAK(); }()
#endif

// call std::abort
#ifdef ATT_DEBUG
#    define ATT_ABORT() static_cast<void>(ATT_DEBUG_BREAK_EXPR(), std::abort())
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

#if defined(ATT_COMP_MSVC)
#   define ATT_ALWAYS_INLINE(p_proto) __forceinline p_proto
#elif defined(ATT_COMP_GCC) || defined(ATT_COMP_CLANG)
#   define ATT_ALWAYS_INLINE(p_proto) p_proto __attribute__((always_inline))
#else
#   define ATT_ALWAYS_INLINE(p_proto) p_proto
#endif

#define ATT_DETAIL_PRAGMA(x) _Pragma(#x)

#ifdef ATT_COMP_MSVC
#    define ATT_DIAG_PUSH() ATT_DETAIL_PRAGMA(warning(push))
#    define ATT_DIAG_POP() ATT_DETAIL_PRAGMA(warning(pop))
#    define ATT_DIAG_MSVC_IGNORE(p_code) ATT_DETAIL_PRAGMA(warning(disable : p_code))
#    define ATT_DIAG_GCC_IGNORE(p_str)
#elif defined(ATT_COMP_CLANG) || defined(ATT_COMP_GCC)
#    define ATT_DIAG_PUSH() ATT_DETAIL_PRAGMA(GCC diagnostic push)
#    define ATT_DIAG_POP() ATT_DETAIL_PRAGMA(GCC diagnostic pop)
#    define ATT_DIAG_MSVC_IGNORE(p_code)
#    define ATT_DIAG_GCC_IGNORE(p_str) ATT_DETAIL_PRAGMA(GCC diagnostic ignored #p_str)
#else
#    define ATT_DIAG_PUSH()
#    define ATT_DIAG_POP()
#    define ATT_DIAG_MSVC_IGNORE(p_code)
#    define ATT_DIAG_GCC_IGNORE(p_str)
#endif

namespace utils
{

[[noreturn]] inline void Unreachable() noexcept
{
#ifdef ATT_DEBUG
    ATT_DEBUG_BREAK();
#endif
    ATT_DETAIL_UNREACHABLE();
}

} // namespace utils
