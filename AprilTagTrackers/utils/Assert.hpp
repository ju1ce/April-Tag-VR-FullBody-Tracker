#pragma once

#include "Cross.hpp"
#include "Log.hpp"

#ifdef ATT_DEBUG
#    include <exception>
#endif

#ifdef ATT_DEBUG
#    define ATT_ASSERT(p_expr, ...)                                                                           \
        ((!!(p_expr)) ? (true)                                                                                \
                      : (::utils::LogPrelude(::utils::LogTag::Assert, __FILE__, __LINE__),                    \
                            ::utils::LogValues(__VA_ARGS__),                                                  \
                            ::utils::LogValues('\n', "ATT_ASSERT( ", #p_expr, " ) in ", ATT_PRETTY_FUNCTION), \
                            ::utils::LogEnd(), ATT_ABORT(), false))
#else
#    define ATT_ASSERT(p_expr, ...) ATT_NOOP()
#endif

#ifdef ATT_DEBUG
#    define ATT_FATAL_EXCEPTION(p_throwExpr, p_context)            \
        do                                                         \
        {                                                          \
            try                                                    \
            {                                                      \
                (p_throwExpr);                                     \
            }                                                      \
            catch (const std::exception& exc)                      \
            {                                                      \
                ATT_LOG_ERROR(p_context, ": ", exc.what());        \
                ATT_ABORT();                                       \
            }                                                      \
            catch (...)                                            \
            {                                                      \
                ATT_LOG_ERROR(p_context, ": malformed exception"); \
                ATT_ABORT();                                       \
            }                                                      \
            ATT_LOG_ERROR(p_context, ": expected exception");      \
            ATT_ABORT();                                           \
        } while (false)
#else
#    define ATT_FATAL_EXCEPTION(p_throwExpr, p_context) ATT_ABORT();
#endif
