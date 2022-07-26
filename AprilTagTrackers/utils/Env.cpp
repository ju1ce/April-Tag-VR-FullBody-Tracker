#include "Env.hpp"

#include <doctest/doctest.h>

#include <cstdlib>
#include <string_view>

namespace
{

constexpr bool StringToBool(std::string_view value)
{
    if (value.empty())
        return false;
    if (value.size() == 1 && value[0] == '0')
        return false;
    return value[0] != 'f' && value[0] != 'F';
}

} // namespace

namespace utils
{

std::thread::id GetThisThreadID()
{
    thread_local const std::thread::id thisThreadID = std::this_thread::get_id();
    return thisThreadID;
}

bool IsMainThread()
{
    thread_local const bool isMainThread = details::MAIN_THREAD_ID == GetThisThreadID();
    return isMainThread;
}

double GetRuntimeSeconds()
{
    return std::chrono::duration<double>(system_clock::now() - GetAppStartTimePoint()).count();
}

std::optional<std::string> EnvVars::GetVarAsString(const std::string& key)
{
    if (!IsMainThread()) std::abort();
    const char* val = std::getenv(key.c_str()); // NOLINT(concurrency-mt-unsafe)
    if (val == nullptr) return std::nullopt;
    return std::string(val);
}

bool EnvVars::GetVarAsBool(const std::string& key)
{
    if (auto envVar = GetVarAsString(key))
    {
        return StringToBool(*envVar);
    }
    return false;
}

} // namespace utils
