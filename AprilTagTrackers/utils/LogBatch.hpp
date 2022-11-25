#pragma once

#include "Log.hpp"
#include "SteadyTimer.hpp"
#include "StringLiteral.hpp"

#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

#if ATT_LOG_LEVEL >= ATT_LOG_LEVEL_DEBUG
#    define ATT_LOG_BATCH(p_name, p_value) ::utils::detail::LogBatch(__FILE__, __LINE__, p_name, p_value)
#else
#    define ATT_LOG_BATCH(p_name, p_value) ATT_NOOP()
#endif

namespace utils::detail
{

struct LogBatchEntry
{
    std::ostringstream buffer{};
    utils::SteadyTimer timer{};
    std::size_t previous = 0xFFFFFFFFU;
    int repeat = 0;

    void AppendRepeat()
    {
        if (repeat > 0)
        {
            buffer << " x" << repeat;
            repeat = 0;
        }
    }
};

template <typename T>
inline void LogBatch(std::string_view filePath, int line, StringLiteral name, const T& value)
{
    static std::unordered_map<std::string_view, LogBatchEntry> logBatches{};
    static std::mutex logBatchesMutex{};

    const std::lock_guard lock{logBatchesMutex};
    auto& entry = logBatches[name.view()];

    const std::size_t hash = std::hash<T>{}(value);
    if (entry.previous == hash)
    {
        ++entry.repeat;
    }
    else
    {
        entry.AppendRepeat();
        entry.buffer << ',' << value;
        entry.previous = hash;
    }

    constexpr std::size_t maxSizeToLog = 200;
    constexpr auto maxTimeToLog = utils::Seconds(15);
    const auto now = utils::SteadyTimer::Now();
    if (entry.buffer.view().size() < maxSizeToLog && entry.timer.Get(now) < maxTimeToLog)
    {
        return;
    }
    entry.timer.Restart(now);

    entry.AppendRepeat();
    ::utils::LogPrelude(LogTag::Debug, filePath, line);
    ::utils::LogValues(name.view(), ": ", entry.buffer.view());
    ::utils::LogEnd();
    entry.buffer.str("");
    entry.previous = 0xFFFFFFFFU;
    entry.repeat = 0;
}

} // namespace utils::detail
