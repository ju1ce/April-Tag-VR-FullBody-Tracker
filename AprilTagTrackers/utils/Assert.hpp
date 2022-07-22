#pragma once

#include "utils/Cross.hpp"
#include "utils/Log.hpp"

#ifdef ATT_DEBUG
#  include <stdexcept>
#endif

#ifdef ATT_DEBUG
#  define ATT_ASSERT(p_expr, ...) \
    ((!!(p_expr)) ? (true) : (ATT_DETAILS_LOG_ASSERT(p_expr, ##__VA_ARGS__), ATT_ABORT(), false))
#else
#  define ATT_ASSERT(p_expr, ...) ATT_NOOP()
#endif

#define ATT_FATAL_EXCEPTION(p_throwExpr, p_context)      \
  do                                                     \
  {                                                      \
    try                                                  \
    {                                                    \
      (p_throwExpr);                                     \
    }                                                    \
    catch (const std::exception& exc)                    \
    {                                                    \
      ATT_LOG_ERROR(p_context, ": ", exc.what());        \
      ATT_ABORT();                                       \
    }                                                    \
    catch (...)                                          \
    {                                                    \
      ATT_LOG_ERROR(p_context, ": malformed exception"); \
      ATT_ABORT();                                       \
    }                                                    \
    ATT_LOG_ERROR(p_context, ": expected exception");    \
    ATT_ABORT();                                         \
  } while (false)

void hi()
{
    ATT_ASSERT(true, "higu");
}
