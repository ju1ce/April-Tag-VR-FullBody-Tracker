#pragma once

#include "SemVer.h"
#include "utils/Macros.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <thread>

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

} // namespace detail

/// time point of application startup
inline system_clock::time_point GetAppStartTimePoint() { return detail::APP_START_TP; }
/// directory application was started in
inline const fs::path& GetRuntimeDir() { return detail::RUNTIME_DIR; }
inline fs::path GetLogsDir() { return GetRuntimeDir() / "logs"; }
inline fs::path GetConfigDir() { return GetRuntimeDir() / "config"; }
inline fs::path GetLocalesDir() { return GetRuntimeDir() / "locales"; }
/// BridgeDriver version
constexpr SemVer GetBridgeDriverVersion() { return detail::BRIDGE_DRIVER_VERSION; }

/// thread id of the callsite thread
std::thread::id GetThisThreadID();
/// is the callsite thread the main thread
bool IsMainThread();
/// runtime duration in seconds
double GetRuntimeSeconds();

class EnvVars
{
public:
    bool IsRedirectConsoleToFile() const { return !logStderr; }

private:
    /// get an environment variable, values are from application start and do not change
    static std::optional<std::string> GetVarAsString(const std::string& key);
    /// the variable not existing is considered falsy
    static bool GetVarAsBool(const std::string& key);

    const bool logStderr = GetVarAsBool("ATT_LOG_STDERR");
};

} // namespace utils
