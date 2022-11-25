#include "Env.hpp"

#include <cstdlib>
#include <mutex>
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

std::thread::id GetThisThreadID() noexcept
{
    thread_local const std::thread::id thisThreadID = std::this_thread::get_id();
    return thisThreadID;
}

bool IsMainThread() noexcept
{
    thread_local const bool isMainThread = detail::MAIN_THREAD_ID == GetThisThreadID();
    return isMainThread;
}

double GetRuntimeSeconds() noexcept
{
    return std::chrono::duration<double>(system_clock::now() - GetAppStartTimePoint()).count();
}

void RegisterThisThreadName(StringLiteral name) noexcept
{
    const std::lock_guard lock{detail::THREAD_NAME_MAP_MUTEX};
    detail::THREAD_NAME_MAP[GetThisThreadID()] = name.view();
}

std::optional<std::string_view> GetThisThreadName() noexcept
{
    const std::lock_guard lock{detail::THREAD_NAME_MAP_MUTEX};
    const auto it = detail::THREAD_NAME_MAP.find(GetThisThreadID());
    if (it == detail::THREAD_NAME_MAP.end()) return std::nullopt;
    return it->second;
}

std::optional<std::string> EnvVars::GetVarAsString(const std::string& key) noexcept
{
    if (!IsMainThread()) std::abort();
#pragma warning(suppress : 4996) // msvc warns for getenv, but there isn't a better option
    const char* val = std::getenv(key.c_str()); // NOLINT
    if (val == nullptr) return std::nullopt;
    return std::string(val);
}

bool EnvVars::GetVarAsBool(const std::string& key) noexcept
{
    if (auto envVar = GetVarAsString(key))
    {
        return StringToBool(*envVar);
    }
    return false;
}

std::optional<fs::path> EnvVars::GetVarAsPath(const std::string& key) noexcept
{
    if (auto pathStr = GetVarAsString(key))
    {
        try
        {
            const auto path = fs::path(*std::move(pathStr));
            return path.lexically_normal();
        }
        catch (const std::exception&)
        {
        }
    }
    return std::nullopt;
}

} // namespace utils
