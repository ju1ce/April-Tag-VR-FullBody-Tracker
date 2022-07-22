#pragma once

#include <doctest/doctest.h>

#include <string_view> // IWYU pragma: keep

// compiler detection
#if defined(__clang__)
#  define ATT_COMP_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
#  define ATT_COMP_GCC 1
#elif defined(_MSC_VER)
#  define ATT_COMP_MSVC 1
#else
#  define ATT_COMP_UNKNOWN 1
#endif

// os detection
#if defined(_WIN32) || defined(__WINDOWS__)
#  define ATT_OS_WINDOWS 1
#elif defined(__APPLE__) || defined(__MACH__)
#  define ATT_OS_MACOS 1
#elif defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#  define ATT_OS_LINUX 1
#else
#  define ATT_OS_UNKNOWN 1
#endif

// get function signature
#if defined(ATT_COMP_CLANG) || defined(ATT_COMP_GCC)
#  define ATT_PRETTY_FUNCTION std::string_view(static_cast<const char* const>(__PRETTY_FUNCTION__))
#elif defined(ATT_COMP_MSVC)
#  define ATT_PRETTY_FUNCTION std::string_view(static_cast<const char* const>(__FUNCSIG__))
#else
#  define ATT_PRETTY_FUNCTION std::string_view(static_cast<const char* const>("<unknown function>()"))
#endif

// trigger debugger breakpoint
#define ATT_BREAKPOINT() DOCTEST_BREAK_INTO_DEBUGGER()

// debug break or abort
#ifdef ATT_DEBUG
#  define ATT_ABORT() ATT_BREAKPOINT()
#else
#  include <cstdlib> // IWYU pragma: keep
#  define ATT_ABORT() std::abort()
#endif

// optimize out unreachable code
#ifdef ATT_DEBUG
#  define ATT_UNREACHABLE() ATT_ABORT()
#else
#  if defined(ATT_COMP_CLANG) || defined(ATT_COMP_GCC)
#    define ATT_UNREACHABLE() __builtin_unreachable()
#  elif defined(ATT_COMP_MSVC)
#    define ATT_UNREACHABLE() __assume(false)
#  else
#    include <cassert>
#    define ATT_UNREACHABLE() assert(false)
#  endif
#endif
