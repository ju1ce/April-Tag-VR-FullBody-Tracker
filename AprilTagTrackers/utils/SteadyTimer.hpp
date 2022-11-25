#pragma once

#include "utils/Assert.hpp"

#include <chrono>

using std::chrono::duration_cast;

namespace utils
{

using NanoS = std::chrono::nanoseconds;
using MilliS = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;
using FSeconds = std::chrono::duration<double>;

template <typename TDuration>
constexpr FSeconds PerSecond(const TDuration duration)
{
    ATT_ASSERT(duration > TDuration(0));
    using FloatDuration = std::chrono::duration<double, typename TDuration::period>;
    constexpr auto oneSec = duration_cast<FloatDuration>(Seconds(1)).count();
    return FSeconds(oneSec / static_cast<double>(duration.count()));
}
class SteadyTimer
{
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = NanoS;

    static TimePoint Now() { return Clock::now(); }

    SteadyTimer() noexcept : mStart(Now()) {}
    Duration Get(const TimePoint now) const
    {
        return duration_cast<Duration>(now - mStart);
    }
    /// get duration since last restart
    Duration Get() const { return Get(Now()); }
    /// reuse a call to Now()
    Duration Restart(const TimePoint now)
    {
        auto difference = Get(now);
        mStart = now;
        return difference;
    }
    /// get current duration, and restart timer
    Duration Restart() { return Restart(Now()); }

private:
    TimePoint mStart{};
};

}; // namespace utils
