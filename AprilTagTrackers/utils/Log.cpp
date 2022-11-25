#include "Log.hpp"

#include "Env.hpp"

#include <iomanip>

namespace utils
{

void LogPrelude(LogTag tag) noexcept
{
    LogValues('[',
              detail::LogTagToString(tag),
              ": ");
    if (const auto threadName = GetThisThreadName())
    {
        LogValues(*threadName);
    }
    else
    {
        LogValues(GetThisThreadID());
    }
    LogValues(' ',
              std::fixed, std::setprecision(4),
              GetRuntimeSeconds(),
              std::defaultfloat,
              "s] ");
}

void LogPrelude(LogTag tag, std::string_view filePath, int line) noexcept
{
    LogPrelude(tag);
    if (filePath.empty()) return;

    if (EnvVars::LOG_ROOT)
    {
        try
        {
            const auto path = fs::path(filePath);
            const auto relative = path.lexically_relative(*EnvVars::LOG_ROOT);
            if (!relative.empty())
            {
                LogValues("(./", relative.generic_string());
                if (line >= 1) LogValues(':', line);
                LogValues(") ");
                return;
            }
        }
        catch (const std::exception&)
        {
            // fall through
        }
    }

    LogValues('(', filePath);
    if (line >= 1) LogValues(':', line);
    LogValues(") ");
}

} // namespace utils
