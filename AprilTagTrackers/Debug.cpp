#include "Debug.h"

namespace Debug
{
const std::thread::id mainThreadID = std::this_thread::get_id();

#if ATT_LOG_LEVEL > 0
const std::chrono::system_clock::time_point appStartTimePoint = std::chrono::system_clock::now();
#endif
} // namespace Debug
