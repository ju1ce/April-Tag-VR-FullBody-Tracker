#pragma once

#include "utils/Cross.hpp"
#include "utils/Log.hpp"

#ifdef ATT_DEBUG
#  define ATT_ASSERT(p_expr, ...) \
    ((!!(p_expr)) ? (true) : (ATT_DETAILS_LOG_ASSERT(p_expr, ##__VA_ARGS__), ATT_ABORT(), false))
#else
#  define ATT_ASSERT(p_expr, ...) ATT_NOOP()
#endif
