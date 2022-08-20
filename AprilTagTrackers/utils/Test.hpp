#pragma once

#ifndef ATT_TESTING
#    define DOCTEST_CONFIG_DISABLE
#endif
#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#include <doctest/doctest.h>

// more info: https://github.com/doctest/doctest/tree/master/doc/markdown

/// declare a test case, behaves like function body e.g. TEST_CASE("foo") { foo; bar; }
#define TEST_CASE(name) DOCTEST_TEST_CASE(name)
/// var args will be concatenated
/// non strings will be stringified if possible, only allows lvalues as they get stringified at the end of the test case
/// this gives the benefit of allowing most doctest macros in loops, without spamming logs or affecting performance
#define LOG_INFO(...) DOCTEST_INFO(__VA_ARGS__)
/// equivalent to LOG_INFO(#x, " := ", x)
#define CAPTURE(x) DOCTEST_CAPTURE(x)
/// require an expression to be true, if false, the test case will exit and be marked as failed
/// decomposes binary and unary expressions and captures their values
/// but is limited to simpler expressions, and cannot decompose unary ! operator
#define REQUIRE(...) DOCTEST_REQUIRE(__VA_ARGS__)
/// REQUIRE with message, see LOG_INFO
#define REQUIRE_M(expr, ...) DOCTEST_REQUIRE_MESSAGE(expr, __VA_ARGS__)
/// similar to REQUIRE but the test case will continue, and still be marked as failed
#define CHECK(...) DOCTEST_CHECK(__VA_ARGS__)
/// CHECK with message, see LOG_INFO
#define CHECK_M(expr, ...) DOCTEST_CHECK_MESSAGE(expr, __VA_ARGS__)
