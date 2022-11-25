#pragma once

#include "Log.hpp"

#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>

namespace utils
{

namespace detail
{

template <typename... TArgs>
inline std::string FormatError(const TArgs&... args) noexcept
{
    try
    {
        std::ostringstream ss{}; // NOLINT(misc-const-correctness)
        ((ss << args), ...);
        return ss.str();
    }
    catch (...)
    {
        // ignore any exceptions from formatting, don't want to lose the exception that is being made
        return {};
    }
}

} // namespace detail

struct Error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

template <typename TError = Error, typename... TArgs>
struct MakeError : TError
{
    template <int N>
    // NOLINTNEXTLINE(modernize-avoid-c-arrays): deduce string literal
    explicit MakeError(const char (&msg)[N], const std::source_location location = std::source_location::current())
        : TError(msg)
    {
        ATT_LOG_ERROR_AT(location.file_name(), location.line(), this->what());
    }
    explicit MakeError(const std::string& msg, const std::source_location location = std::source_location::current())
        : TError(msg)
    {
        ATT_LOG_ERROR_AT(location.file_name(), location.line(), this->what());
    }
    explicit MakeError(const TArgs&... args, const std::source_location location = std::source_location::current())
        : TError(detail::FormatError(args...))
    {
        ATT_LOG_ERROR_AT(location.file_name(), location.line(), this->what());
    }
};

template <typename TError = Error, typename... TArgs>
MakeError(const TArgs&...) -> MakeError<TError, TArgs...>;

template <typename TError = Error, typename... TArgs>
    requires std::is_base_of_v<Error, TError>
struct Throw
{
    template <int N>
        requires(N >= 1)
    // NOLINTNEXTLINE(modernize-avoid-c-arrays): deduce string literal
    [[noreturn]] explicit Throw(const char (&msg)[N], const std::source_location location = std::source_location::current())
    {
        ATT_LOG_ERROR_AT(location.file_name(), location.line(), this->what());
        throw TError(msg);
    }
    [[noreturn]] explicit Throw(const std::string& msg, const std::source_location location = std::source_location::current())
    {
        ATT_LOG_ERROR_AT(location.file_name(), location.line(), this->what());
        throw TError(msg);
    }
    [[noreturn]] explicit Throw(const TArgs&... args, const std::source_location location = std::source_location::current())
    {
        ATT_LOG_ERROR_AT(location.file_name(), location.line(), this->what());
        throw TError(detail::FormatError(args...));
    }
};

template <typename TError = Error, typename... TArgs>
Throw(const TArgs&...) -> Throw<TError, TArgs...>;

} // namespace utils
