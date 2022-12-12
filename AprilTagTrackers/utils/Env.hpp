#pragma once

#include "SemVer.h"
#include "StringLiteral.hpp"
#include "utils/Macros.hpp"

#include <chrono>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

namespace utils
{
using system_clock = std::chrono::system_clock;
namespace fs = std::filesystem;

namespace detail
{

/// this_thread::get_id called during static initialization
inline const std::thread::id MAIN_THREAD_ID = std::this_thread::get_id();
/// time point of application startup
inline const system_clock::time_point APP_START_TP = system_clock::now();
/// directory application was started in
inline const fs::path RUNTIME_DIR = fs::current_path();
/// BridgeDriver version
inline constexpr SemVer BRIDGE_DRIVER_VERSION = SemVer::Parse(ATT_STRINGIZE(ATT_DRIVER_VERSION));

inline std::unordered_map<std::thread::id, std::string_view> THREAD_NAME_MAP{}; // NOLINT
inline std::mutex THREAD_NAME_MAP_MUTEX{};                                      // NOLINT

} // namespace detail

/// time point of application startup
inline system_clock::time_point GetAppStartTimePoint() noexcept { return detail::APP_START_TP; }
/// directory application was started in
inline const fs::path& GetRuntimeDir() { return detail::RUNTIME_DIR; }
inline fs::path GetBindingsDir() { return GetRuntimeDir() / "bindings"; }
inline fs::path GetLogsDir() { return GetRuntimeDir() / "logs"; }
inline fs::path GetConfigDir() { return GetRuntimeDir() / "config"; }
inline fs::path GetLocalesDir() { return GetRuntimeDir() / "locales"; }
/// BridgeDriver version
constexpr SemVer GetBridgeDriverVersion() { return detail::BRIDGE_DRIVER_VERSION; }

/// thread id of the callsite thread
std::thread::id GetThisThreadID() noexcept;
/// is the callsite thread the main thread
bool IsMainThread() noexcept;
/// runtime duration in seconds
double GetRuntimeSeconds() noexcept;
/// is the callsite from the same thread as the passed thread
inline bool IsThread(const std::thread& target) noexcept { return GetThisThreadID() == target.get_id(); }

/// set an identifying name for this thread
void RegisterThisThreadName(StringLiteral name) noexcept;
/// get the identifying name if it was set
std::optional<std::string_view> GetThisThreadName() noexcept;

class EnvVars
{
    /// get an environment variable, values are from application start and do not change
    static std::optional<std::string> GetVarAsString(const std::string& key) noexcept;
    /// the variable not existing is considered falsy
    static bool GetVarAsBool(const std::string& key) noexcept;
    static std::optional<fs::path> GetVarAsPath(const std::string& key) noexcept;

public:
    bool IsRedirectConsoleToFile() const { return !logStderr; }
    static inline const std::optional<fs::path> LOG_ROOT = GetVarAsPath("ATT_LOG_ROOT");

private:
    const bool logStderr = GetVarAsBool("ATT_LOG_STDERR");
};

} // namespace utils
